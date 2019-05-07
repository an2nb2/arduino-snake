#include "LedControl.h"

// Initialize MAX72XX
LedControl lc = LedControl(12,10,11,1);

const unsigned short btnLeft  = 2;
const unsigned short btnRight = 3;
const unsigned short btnUp    = 4;
const unsigned short btnDown  = 5;

/*
  Sizes of the board. For the MAX72XX it's 8x8.
*/
const unsigned short boardHeight = 8;
const unsigned short boardWidth = 8;

/*
  Game settings.
*/
const unsigned long renderDelay=300;
const unsigned short snakeInitialSize = 2;
const unsigned short snakeMaxSize = 10;

/*
  Game types
*/
enum Directions {
  Right,
  Left,
  Up,
  Down
};
struct Point {
  int x, y;
};

/*
  Global game objects.
*/

Point snake[snakeMaxSize];
int snakeCurrentSize;
Point prey;

Directions direction;

unsigned long timer;

void setup() {
  Serial.begin(9600);

  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0, 8);

  resetGame();
}

void resetGame() {
  /* and clear the display */
  lc.clearDisplay(0);

  /*
   Setup buttons pins
  */
  pinMode(btnLeft, INPUT_PULLUP);
  pinMode(btnRight, INPUT_PULLUP);
  pinMode(btnUp, INPUT_PULLUP);
  pinMode(btnDown, INPUT_PULLUP);

  /*
   Setup initial state and render snake.
  */
  snakeCurrentSize = snakeInitialSize;
  direction = Up;

  for(int i = 0; i < snakeCurrentSize; i++) {
    snake[i].x = (snakeCurrentSize - 1) - i;
    snake[i].y = 0;

    lc.setLed(0, snake[i].x, snake[i].y, true);
  }

  timer = millis();

  generatePrey();
}

Point getNextPosition() {
  Point nextPosition = {snake[0].x, snake[0].y};

  switch(direction) {
  case Up:
    if ((snake[0].x + 1) >= boardHeight) {
      nextPosition.x = 0;
    } else {
      nextPosition.x = snake[0].x + 1;
    }
    break;
  case Down:
    if ((snake[0].x - 1) < 0) {
      nextPosition.x = boardHeight - 1;
    } else {
      nextPosition.x = snake[0].x - 1;
    }
    break;
  case Right:
    if ((snake[0].y - 1) < 0) {
      nextPosition.y = boardWidth - 1;
    } else {
      nextPosition.y = snake[0].y - 1;
    }
    break;
  case Left:
    if ((snake[0].y + 1) >= boardWidth) {
      nextPosition.y = 0;
    } else {
      nextPosition.y = snake[0].y + 1;
    }
    break;
  }

  return nextPosition;
}

void shiftSnakeTo(Point nextPosition) {
  for(int i = 0; i < snakeCurrentSize; i++) {
    Point tmp = snake[i];

    snake[i] = nextPosition;

    nextPosition = tmp;
  }
}

void updateDirection() {
  if (digitalRead(btnLeft) == LOW) {
    if (direction != Right) {
      direction = Left;
    }
  } else if (digitalRead(btnRight) == LOW) {
    if (direction != Left) {
      direction = Right;
    }
  } else if (digitalRead(btnUp) == LOW) {
    if (direction != Down) {
      direction = Up;
    }
  } else if (digitalRead(btnDown) == LOW) {
    if (direction != Up) {
      direction = Down;
    }
  }
}

void generatePrey() {
  bool found = false;

  while(!found) {
    prey.x = rand() % 8;
    prey.y = rand() % 8;

    found = true;

    for(int i = 0; i < snakeCurrentSize; i++) {
      if (snake[i].x == prey.x && snake[i].y == prey.y) {
        found = false;
        break;
      }
    }
  }

  lc.setLed(0, prey.x, prey.y, true);
}

void eatPrey() {
  snakeCurrentSize++;

  shiftSnakeTo(prey);

  generatePrey();
}

void moveSnakeTo(Point nextPosition) {
  /* Turn LED on new position */
  lc.setLed(0, nextPosition.x, nextPosition.y, true);
  /* Turn off old tail LED */
  lc.setLed(0, snake[snakeCurrentSize - 1].x, snake[snakeCurrentSize - 1].y, false);

  shiftSnakeTo(nextPosition);
}

bool metSelf(Point nextPosition) {
  for(int i = 0; i < snakeCurrentSize; i++) {
    if (snake[i].x == nextPosition.x && snake[i].y == nextPosition.y) {
      return true;
    }
  }

  return false;
}

void loop() {
  bool selfEaten = false;
  bool won = false;

  while(!selfEaten || !won) {
    if( millis() - timer > renderDelay ) {
      Point nextPosition = getNextPosition();

      if (nextPosition.x == prey.x && nextPosition.y == prey.y) {
        if (snakeMaxSize <= snakeCurrentSize) {
          won = true;
          break;
        } else {
          eatPrey();
        }
      } else {
        if (metSelf(nextPosition)) {
          selfEaten = true;
          break;
        } else {
          moveSnakeTo(nextPosition);
        }
      }

      timer = millis();
    }

    updateDirection();
  }

  if (selfEaten) {
    Serial.print("You lost!");
  } else if (won) {
    Serial.print("You won!");
  }

  delay(10000);

  resetGame();
}
