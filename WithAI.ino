#include <Wire.h>
#include "Adafruit_MPR121.h"
#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif
#include <Adafruit_NeoPixel.h>

#include "Utilisateur.h"
#define PIXEL_PIN   3  // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 9  // Number of NeoPixels


Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_RGB + NEO_KHZ800); //declaration de la bande neopixel
Adafruit_MPR121 cap = Adafruit_MPR121(); //declaration de la carte tactile

//enregistrement contact
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

bool victoire = 0;
bool choixPremier = false; //si false joueur 1 en premier
bool joueur = false; //tour du joueur
bool firstMove = 1; //pour jeu contre IA
bool secondMove = 0; //pour IA


const uint8_t ledPosition[3][3] = { //adresse de chaque LED dans le tableau de jeu (dépendant du montage)
  {0, 1, 2},
  {5, 4, 3},
  {6, 7, 8}
};
uint32_t colorGrid[3][3] = { //couleur des LED
  {(0, 0, 0), (0, 0, 0), (0, 0, 0)},
  {(0, 0, 0), (0, 0, 0), (0, 0, 0)},
  {(0, 0, 0), (0, 0, 0), (0, 0, 0)}
};

//variables pour pression changement de mode
//mode 0
unsigned long pressedTime;
unsigned long releasedTime;
//mode 1
unsigned long pressedTime2 = 0;
unsigned long releasedTime2 = 0;
//mode 2
unsigned long pressedTime3 = 0;
unsigned long releasedTime3 = 0;
//changement ordre de joueur
unsigned long pressedTime4 = 0;
unsigned long releasedTime4 = 0;
//reset
unsigned long resetTime = 0;
unsigned long resetEndTime = 0;


uint8_t mode = 0; //mode de jeu actuel
bool touchMode = 0; //mis a true lorsque l'on effectue une action : ex changement de mode

//couleurs
uint32_t vide = (0, 0, 0); //LED Eteinte
uint32_t drawColor = strip.Color(drawColor1[0], drawColor1[1], drawColor1[2]); //affichage match nul
uint32_t modeColor = strip.Color(modeColor1[0], modeColor1[1], modeColor1[2]); //affichage de changement de mode
uint32_t diceColor = strip.Color(diceColor1[0], diceColor1[1], diceColor1[2]); //affichage dé
uint32_t joueur1 = strip.Color(couleur_joueur1[0], couleur_joueur1[1], couleur_joueur1[2]); //couleur joueur 1 Tic tac toe
uint32_t joueur2 = strip.Color(couleur_joueur2[0], couleur_joueur2[1], couleur_joueur2[2]); // couleur Joueur 2 Tic Tac Toe
uint32_t diceThrowAnim = strip.Color(diceThrowAnim1[0], diceThrowAnim1[1], diceThrowAnim1[2]);

//fonctions
void playTicTacToe();
void win();
void draw();
//fonctions pour IA
void aITurn();
bool checkMovesLeft (uint32_t gBoard[3][3]);
int minimax (uint32_t (&gBoard)[3][3], int depth, bool isMax);
int evaluate (uint32_t gBoard[3][3]);
//
void throwDice();
void changeMode(uint16_t bouton1, uint16_t bouton2, unsigned long &pressedTime_, unsigned long &releasedTime_, uint8_t modetype);
void blinking(uint32_t grille[3][3], uint8_t blinkNumber = 4, uint32_t interval = 250);
void animMode(uint8_t Mode);
void clearGrid();
void reset();


void setup() 
{
  //Serial.begin(9600); //pour debug
  strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
  strip.show();  // Initialize all pixels to 'off'
  cap.begin(0x5A);
  strip.setBrightness(brightness); //ajuste luminosité des LEDs
}


void loop() {

  // Get the currently touched pads
  currtouched = cap.touched();
    
    changeMode(ledPosition[0][0],ledPosition[0][1], pressedTime, releasedTime, 0); //mode 0
    changeMode(ledPosition[1][0],ledPosition[1][1], pressedTime2, releasedTime2, 1); //mode 1
    changeMode(ledPosition[2][0],ledPosition[2][1], pressedTime3, releasedTime3, 2); //mode 2
    changeMode(ledPosition[2][0],ledPosition[2][2], pressedTime4, releasedTime4, 3); //changement ordre joueur
    changeMode(ledPosition[2][0],ledPosition[0][2], resetTime, resetEndTime, 4); //reset
    
    if (!touchMode and ((mode == 0) or (mode == 2)))  
    {
      playTicTacToe();
    }

    if ((mode == 1) and !touchMode) 
    {
      throwDice();
    }
  

    if (!currtouched) 
    {
      touchMode = 0;
    }

  lasttouched = currtouched;

}

void changeMode(uint16_t bouton1, uint16_t bouton2, unsigned long &pressedTime_, unsigned long &releasedTime_, uint8_t modetype) 
{
  if (!touchMode) 
  {
    if ((currtouched & _BV(bouton1)) and (currtouched & _BV(bouton2)) && (!(lasttouched & _BV(bouton1))) or !(lasttouched & _BV(bouton2))) 
    {
    pressedTime_ = millis();
    }
      if (currtouched & _BV(bouton1) && currtouched & _BV(bouton2) && lasttouched & _BV(bouton1) && lasttouched & _BV(bouton2))  
      {
        releasedTime_ = millis();
        if (releasedTime_ - pressedTime_ > 1000) 
        {
           touchMode = 1;
          if(modetype == 3) 
          {
            choixPremier = !choixPremier;
            
          } else if(modetype == 4) 
          {
             reset();
             return;
          } else {
            mode = modetype;
          }
          reset();
          animMode(modetype); 
       }
     }
   }
}
  
  
  
    void animMode(uint8_t Mode) { //animation de changement de mode
     uint32_t wait = 100;
     uint32_t waitMode = 1000;
      switch(Mode) {
        case 0 : //affiche un T
          colorGrid[0][0] = modeColor;
          colorGrid[0][1] = modeColor;
          colorGrid[0][2] = modeColor;
          colorGrid[1][1] = modeColor;
          colorGrid[2][1] = modeColor;
          affiche();
          delay(waitMode);
          clearGrid();
          affiche();
        break;
        case 1 : //affiche un D
          for (uint8_t i = 0; i < 3; i++) 
          {
            for (uint8_t j = 0; j < 3; j++) 
            {
              colorGrid[i][j] = modeColor;
            }
          }
          colorGrid[0][2] = vide;
          colorGrid[2][2] = vide;
          colorGrid[1][1] = vide;
          affiche();
          delay(waitMode);
          clearGrid();
          affiche();
        break;
        case 2 :
        colorGrid[0][0] = modeColor;

        colorGrid[0][2] = modeColor;

        colorGrid[2][1] = modeColor;
        affiche();
        delay(waitMode);
        clearGrid();
        affiche();
        break;
        case 3 :
          uint32_t color;
          
          if(choixPremier) 
          {
           color = joueur2;
          } else {
           color = joueur1;
          }
          colorGrid[0][0]= color;
          affiche();
          delay(wait);
          colorGrid[0][1] = color;
          colorGrid[1][0] = color;
          affiche();
          delay(wait);
          colorGrid[0][0] = vide;
          colorGrid[0][2] = color;
          colorGrid[1][1] = color;
          colorGrid[2][0] = color;
          affiche();
          delay(wait);
           colorGrid[0][1] = vide;
          colorGrid[1][0] = vide;
          colorGrid[1][2] = color;
          colorGrid[2][1] = color;
          affiche();
          delay(wait);
          colorGrid[0][2] = vide;
          colorGrid[1][1] = vide;
          colorGrid[2][0] = vide;
          colorGrid[2][2] = color;
          affiche();
          delay(wait);
           colorGrid[1][2] = vide;
          colorGrid[2][1] = vide;
          affiche();
          delay(wait);
          clearGrid();
          affiche();
        
        break;

        default : 
        clearGrid();
        
      }
    }
  
    void win()   //vérifie s'il y a victoire et fait clignoter la/les lignes gagnantes
    {
      
      //check les lignes
      uint32_t grille[3][3] = {0};
     
      for (uint8_t i = 0; i < 3; i++) 
      {
        if (!(colorGrid[i][0] == (0, 0, 0)) && (colorGrid[i][0] == colorGrid[i][1]) && (colorGrid[i][0] == colorGrid[i][2])) 
        {
          grille[i][0] = colorGrid[i][0];
          grille[i][1] = colorGrid[i][0];
          grille[i][2] = colorGrid[i][0];
         
          victoire = 1;
        }
      }
      //check les colonnes
      for (uint8_t j = 0; j < 3; j++) 
      {
        if (!(colorGrid[0][j] == (0, 0, 0)) && (colorGrid[0][j] == colorGrid[1][j]) && ( colorGrid[0][j] == colorGrid[2][j])) 
        {
          grille[0][j] = colorGrid[0][j];
          grille[1][j] = colorGrid[0][j];
          grille[2][j] = colorGrid[0][j];
         
          victoire = 1;
        }
      }
      //check diagonale 1
      if (!(colorGrid[0][0] == (0, 0, 0)) && (colorGrid[0][0] == colorGrid[1][1]) && ( colorGrid[0][0] == colorGrid[2][2])) 
      {
        grille[0][0] = colorGrid[0][0];
        grille[1][1] = colorGrid[0][0];
        grille[2][2] = colorGrid[0][0];
          
        victoire = 1;
      }
      
      //check diagonale 2

      if (!(colorGrid[2][0] == (0, 0, 0)) && (colorGrid[2][0] == colorGrid[1][1]) && ( colorGrid[2][0] == colorGrid[0][2])) 
      {
        grille[2][0] = colorGrid[2][0];
        grille[1][1] = colorGrid[2][0];
        grille[0][2] = colorGrid[2][0];
          
        victoire = 1;
      }

      if (victoire) 
      {
       blinking(grille);
        reset();
        lasttouched = 0;
        currtouched = 0;
      }
    }
    
  void blinking(uint32_t grille[3][3], uint8_t blinkNumber, uint32_t interval) 
  {
       for(uint8_t i= 0; i< blinkNumber; i++) 
       {
        for (uint8_t j = 0; j < 3; j++) 
        {
            for (uint8_t k = 0; k < 3; k++) 
            {
                  if(grille[j][k] != vide) 
                  {
                      strip.setPixelColor(ledPosition[j][k], grille[j][k]);
                  }
  
            }
          }
          strip.show();
          delay(interval);
         for (uint8_t j = 0; j < 3; j++) 
         {
            for (uint8_t k = 0; k < 3; k++) 
            {
                  if(grille[j][k] != vide) 
                  {
                     strip.setPixelColor(ledPosition[j][k], vide);
                  }
  
            }
          }
          strip.show();    
          delay(interval);
      }
   }


    void reset() 
    {
      clearGrid();
      affiche();
      joueur = choixPremier;
      victoire = 0;
      firstMove = 1;
      secondMove = choixPremier;
    }
 
    void draw()
    {
      if(!checkMovesLeft(colorGrid))
      {
        uint32_t grille[3][3] = {drawColor, drawColor, drawColor, vide, vide, vide, drawColor, drawColor, drawColor};
         clearGrid();
         affiche();
         blinking(grille);
        reset();
      }
    }
    void clearGrid() 
    {
      for (uint8_t i = 0; i < 3; i++) {
        for (uint8_t j = 0; j < 3; j++) {
          colorGrid[i][j] = vide;
        }
      }
    }
    void playTicTacToe() 
    {

      if ((mode == 0) or ((mode == 2 and !joueur) and !victoire)) 
      {
        for (uint8_t i = 0; i < 3; i++) 
        {
          for (uint8_t j = 0; j < 3; j++) 
          {


            if ( !(currtouched & _BV(ledPosition[i][j])) && (lasttouched & _BV(ledPosition[i][j])) && (colorGrid[i][j] == (0, 0, 0)))  //si sensor touché, pas touché avant ET led pas encore allumée
            {
              //Serial.print(i);  Serial.print(j); Serial.println(" touched");

              switch (joueur) {
                case 0:
                  colorGrid[i][j] = joueur1;

                  break;
                  
                case 1:
                  colorGrid[i][j] = joueur2;

                  break;

              }
              affiche();
              joueur = !joueur;
              touchMode = 1;


            }

          }
        }
      } else if ((mode == 2) and joueur and !victoire) {
        aITurn();
        touchMode = 1;
      }

      win(); //check si win + anim si oui
      draw(); //check si draw + anim si oui

    }



    void throwDice() {
      uint16_t wait(50);
      uint32_t color = diceThrowAnim;
      if (!(currtouched ) && (lasttouched )) {
        clearGrid();
        affiche();
        colorGrid[0][0] = color;
        affiche();
        delay(wait);
        colorGrid[0][1] = color;
        affiche();
        delay(wait);
        colorGrid[0][2] = color;
        affiche();
        delay(wait);
        colorGrid[1][2] = color;
        affiche();
        delay(wait);
        colorGrid[2][2] = color;
        affiche();
        delay(wait);
        colorGrid[2][1] = color;
        affiche();
        delay(wait);
        colorGrid[2][0] = color;
        affiche();
        delay(wait);
        colorGrid[1][0] = color;
        affiche();
        delay(wait);
        colorGrid[1][1] = color;
        affiche();
        delay(wait);

        clearGrid();

        uint8_t result = random(1, 7);

        switch (result) {
          case 1 :
            colorGrid[1][1] = diceColor;
            break;
          case 2 :
            colorGrid[0][0] = diceColor;
            colorGrid[2][2] = diceColor;
            break;
          case 3 :
            colorGrid[0][0] = diceColor;
            colorGrid[1][1] = diceColor;
            colorGrid[2][2] = diceColor;
            break;
          case 4:
            colorGrid[0][0] = diceColor;
            colorGrid[0][2] = diceColor;
            colorGrid[2][0] = diceColor;
            colorGrid[2][2] = diceColor;
            break;
          case 5:
            colorGrid[0][0] = diceColor;
            colorGrid[0][2] = diceColor;
            colorGrid[2][0] = diceColor;
            colorGrid[2][2] = diceColor;
            colorGrid[1][1] = diceColor;
            break;
          case 6:
            colorGrid[0][0] = diceColor;
            colorGrid[0][2] = diceColor;
            colorGrid[2][0] = diceColor;
            colorGrid[2][2] = diceColor;
            colorGrid[1][0] = diceColor;
            colorGrid[1][2] = diceColor;
            break;
        }
        affiche();

      }
    }
    void affiche() { //sans paramètre, affiche le tableau de couleur en variable globale
      for (uint8_t i = 0; i < 3; i++) {
        for (uint8_t j = 0; j < 3; j++) {
          strip.setPixelColor(ledPosition[i][j], colorGrid[i][j]);
          strip.show();
        }
      }
      strip.show();
    }

    int minimax (uint32_t (&gBoard)[3][3], int depth, bool isMax)            //Considers the possible plays and returns a value for the current board position
    {

      int score = evaluate(gBoard);

      if (score == 10 || score == -10)                                //If a player has won the game
        return score;

      if (checkMovesLeft(gBoard) == false)                           //If there are no moves left then it must be a draw
        return 0;



      if (isMax)                                                      //If this is the maximizer's turn
      {
        int best = -1000;
        for (uint8_t i = 0; i <= 2; i++)  {                                //Go through each cell
          for (uint8_t j = 0; j <= 2; j++) {
            if (gBoard[i][j] == vide)                                    //If the space hasn't been played
            {
              gBoard[i][j] = joueur1;                                       //Play the move
              best = max (best, minimax(gBoard, depth + 1, !isMax));
              gBoard[i][j] = vide;                                       //Undo the move
            }
          }
        }
        return best;
      }
      else                                                            //If this is the minimizer's turn
      {
        int best = 1000;
        for (uint8_t i = 0; i <= 2 ; i++) {                                 //Go through each cell
          for (uint8_t j = 0; j <= 2; j++) {
            if (gBoard[i][j] == vide)                                    //If the space hasn't been played
            {
              gBoard[i][j] = joueur2;                                       //Play the move
              best = min (best, minimax(gBoard, depth + 1, !isMax));
              gBoard[i][j] = vide;                                       //Undo the move
            }
          }
        }
        return best;
      }
    }

    void aITurn () {
    delay(500);

      uint8_t bestRow = 1;
      uint8_t bestCol = 0;
      Serial.println(firstMove);
      if (firstMove) //si premier coup, joue au hasard
      {
        bool played = false;
        
        while (!played) 
        {

          uint8_t templ = random(0, 3);
          uint8_t tempc = random(0, 3);
          if (colorGrid[templ][tempc] == vide) {
            bestRow = templ;
            bestCol = tempc;
            played = true;

          }
        };
        firstMove = false;
      } else if(secondMove) //si deuxième coup (true quand AI joue en premier)
      {
          bool played = false;
        while (!played) {

          uint8_t templ = random(0, 3);
          uint8_t tempc = random(0, 3);
          if (colorGrid[templ][tempc] == vide) {
            bestRow = templ;
            bestCol = tempc;
            played = true;

          }
        };
        
          secondMove = 0;
      }
      else
      {
        int bestVal = +1000;
         //Go through each cell
        for (uint8_t i = 0; i <= 2; i++) 
        {                                
          for (uint8_t j = 0; j <= 2; j++) 
          {
            if (colorGrid[i][j] == vide)  //If the space hasn't been played                             
            {
              colorGrid[i][j] = joueur2;        //Play the move                      
              int moveVal = minimax(colorGrid, 0, true); 
              Serial.println(moveVal);
              colorGrid[i][j] = vide;  //Undo the move


              if (moveVal < bestVal)
              {
                bestRow = i;
                bestCol = j;
                bestVal = moveVal;
              }
            }
          }
        }
        if (bestVal == 10) //si toutes les situations mènent à défaite de l'IA
        {
          for (uint8_t i = 0; i < 3; i++) 
          {
            for (uint8_t j = 0; j < 3; j++) 
            {
              if (colorGrid[i][j] == vide) 
              {
                colorGrid[i][j] = joueur1; //on joue le prochain coup de l'adversaire
                int secondBest = evaluate(colorGrid);
                colorGrid[i][j] = vide; //on annule
                if (secondBest >= bestVal) //on joue à la place du meilleur coup de l'adversaire
                {
                  bestVal = secondBest;
                  bestRow = i;
                  bestCol = j;
                }
              }
            }
          }
        }
      }



      colorGrid[bestRow][bestCol] = joueur2; //Once all combinations have been explored, play the best move
      joueur = 0; //on redonne la main au joueur 1
      affiche();
    }
    
    int evaluate (uint32_t gBoard[3][3])                                         //Function to evaluate the current board value
    {
      for (uint8_t i = 0; i <= 2; i++)                                          //Loop through all rows and check for a win
      {
        if (gBoard[i][0] == gBoard[i][1] && gBoard[i][1] == gBoard[i][2])
        {
          if (gBoard[i][0] == joueur1)
            return 10;
          else if (gBoard[i][0] == joueur2)
            return -10;
        }
      }
      for (uint8_t j = 0; j <= 2; j++)                                          //Loop through all columns and check for a win
      {
        if (gBoard[0][j] == gBoard[1][j] && gBoard[1][j] == gBoard[2][j])
        {
          if (gBoard[0][j] == joueur1)
            return 10;
          else if (gBoard[0][j] == joueur2)
            return -10;
        }
      }
      if (gBoard[0][0] == gBoard[1][1] && gBoard[1][1] == gBoard[2][2])     //Check first diagonal for a win
      {

        if (gBoard[0][0] == joueur1)
          return 10;
        else if (gBoard[0][0] == joueur2)
          return -10;
      }
      if (gBoard[0][2] == gBoard[1][1] && gBoard[1][1] == gBoard[2][0])     //Check second diagonal for a win
      {
        if (gBoard[0][2] == joueur1)
          return 10;
        else if (gBoard[0][2] == joueur2)
          return -10;
      }
      return 0;
    }
    
    bool checkMovesLeft (uint32_t gBoard[3][3])  //Function to check if there are still empty spaces on the board
    {
      for (uint8_t i = 0; i <= 2 ; i++)   {   //Loop through all 9 board positions - i row, j column
        for (uint8_t j = 0; j <= 2; j++) {
          if (gBoard[i][j] == vide) {   //If an empty space is found, return true
            return true;
          }
        }
      }
      return false;
    }
