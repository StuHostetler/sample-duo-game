/*  The ECE101 Duo Project
    Stuart Hostetler
    Spring 23

    This program recreates the game of Duo, attempting to imitiate the physical game itself to the best of my abilities. 
    It comes included with ASCII graphics, the ability to play with up to 6 players, and that this game will (hopefully) work until the endgame state is reached.
    This is achieved by using several structs representing game features to ease the passing of data objects between functions.
    There are some things that I would of changed or improved further (like the discard function) if I had more time.
*/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

//All 108 cards are created as card objects, that are to remain fixed in their data save for the next card memory address. 
//That gets manuiplated and shifted as the card's moved between lists
typedef struct card_s { //Base cards
    char color[7]; //Unused for ASCII graphics
    int value;
    char action[15]; //Unused for ASCII graphics
    int suit; //1-red 2-blue 3-green 4-yellow 5-rainbow
    int type; //1-face 2-wild num 3-wild suit
    struct card_s *pt; //Next card
} card;

typedef struct player_s { //Player object
    int player_num;
    //bool player_cpu; //If automation is added
    card *hand_head; //First card in hand
    int hand_count;
    int points;
} player;

//A seperate struct for the decks and game elements so they could all be passed as a single object
typedef struct dealer_s { //Dealer control object
    card *draw_head; //First card in deck
    int draw_count;
    card *discard_head; //First card in discard
    int discard_count;
    card *pile_1; //Match piles, they should remain single element only (used cards are moved to discard)
    card *pile_2;
    int pile_1_wild; //If it's a wildcard, use this variable instead to determine the number
    int pile_2_wild;
    int pile_1_rainbow; //If it's a rainbow card, use this variable instead to determine the suit
    int pile_2_rainbow;
} dealer;

//Dos rules:
//Hand seven cards to each player and set up two piles
//Each turn, a player can play a card that matches the number on a pile
//Or can play two cards that equals the current number on a pile
//If the color matches, the player can discard an additional card
//If there is a color match on a two card play, the player discards a card and all other players must draw one card
//When one player wins the round by emptying their hand, they gain points according to what cards the other players have
//Face cards give points equal to their number, rainbow cards give 20 pts, and wild number cards give 40 pts
//Player wins at 200 pts

//DECK MANAGEMENT
void createDeck(dealer *deck);
card *shuffleDeck(card *head, int size); //Return the head of the deck being shuffled
void restockDeck(dealer *deck);
void dealPiles(dealer *deck);
void dealPlayer(dealer *deck, player *hand);
void clearHands(player p[], int max_player, dealer *deck);
void clearPiles(dealer *deck);
//GAME DISPLAY
void displayCards(card *head, int size, int iteration);
void displayPiles(card *left, card *right, int wild_1, int wild_2, int rainbow_1, int rainbow_2);
//card *sortHand(card *head, int size);
//GAME FUNCTIONS
void discardCard(card **hand_head, int *hand_size, int hand_index, card **discard_head, int *discard_size, bool play); //Double pointers so address changes persist
void drawCard(dealer *deck, player *hand);
void drawNextPile(dealer *deck, int tag);
void playToPile(dealer *deck, int tag, player *p, int hand_index);
int testOneCard(player *p, int index, card *pile, int wild, int rainbow);
int testTwoCard(player *p, int index_1, int index_2, card *pile, int wild, int rainbow);
int awardPoints(player p[], int winner, int max_players);


int main() {
    dealer deck;
    player p[6]; //The max six players
    int turn=1, round=1, current_player=0, max_players=0, points_to_win=200; //Game variables
    int input=0, play_input_1=0, play_input_2=0, play_input_3=0; //IO control
    int i=0, result=0, score_earned=0;
    bool game_active=true, new_round=false, turn_active=false, retry_option=true; //Turn loop controls
    //Game_active is main loop state, new_round controls resetting hands, turn_active controls the specific turn's loop state, 

    //--DEALER SETUP--
    deck.pile_1=NULL;
    deck.pile_2=NULL;
    deck.pile_1_wild=0;
    deck.pile_2_wild=0;
    deck.pile_1_rainbow=0;
    deck.pile_2_rainbow=0;
    deck.discard_count=0;
    deck.draw_count=0;
    createDeck(&deck);
    deck.draw_head=shuffleDeck(deck.draw_head,deck.draw_count);

    //--PLAYER SETUP--
    for (i=0;i<6;i++) {
        p[i].player_num=(i+1);
        p[i].hand_head=NULL;
        p[i].hand_count=0;
        p[i].points=0;
    }
    while (max_players<2 || max_players>6) {
        printf("Number of players? (2-6) >");
        scanf("%d",&max_players);
        printf("\n");
    }
    //Deal cards
    for (i=0;i<max_players;i++) {
        dealPlayer(&deck,&p[i]);
        displayCards(p[i].hand_head,p[i].hand_count,0);
    } 
    //Create piles
    dealPiles(&deck);
    //displayPiles(deck.pile_1,deck.pile_2); 

    printf("\n\nThe game is ready to start!\n\n");

    //--MAIN GAME LOOP--
    while (game_active==true) {
        //Turn start!
        //Check if new round is happening
        if (new_round==true) { 
            clearPiles(&deck); //Clear the piles
            for (i=0;i<max_players;i++) { //Clear everyone's hands
                clearHands(p,max_players,&deck);
            }
            restockDeck(&deck); //Refill and reshuffle the deck
            for (i=0;i<max_players;i++) { //Draw new hands
                dealPlayer(&deck,&p[i]);
                displayCards(p[i].hand_head,p[i].hand_count,0);
            } 
            dealPiles(&deck); //Draw new piles
            turn=1;
            round++;
            new_round=false;
        }
        printf("==========\n\n");
        printf("Round %d - Turn %d\nPlayer %d's turn!\nThey currently have %d point(s).\n",round,turn,p[current_player].player_num,p[current_player].points);
        turn_active=true;
        //retry_option=false;

        //What should the player do?
        while (turn_active==true) {

            if (p[current_player].hand_count<1) { //Check if Player has emptied their hand and won the round
                printf("Player %d has won the round!!\n",p[current_player].player_num);
                score_earned=awardPoints(p,current_player,max_players);
                printf("They earned %d points!\n",score_earned);
                p[current_player].points=p[current_player].points+score_earned;
                new_round=true;
                break;
            }

            displayCards(p[current_player].hand_head,p[current_player].hand_count,0);
            displayPiles(deck.pile_1,deck.pile_2,deck.pile_1_wild,deck.pile_2_wild,deck.pile_1_rainbow,deck.pile_2_rainbow); 
            printf("Player %d:\n1-Play a card\n2-Play two cards\n3-Draw and replace a Center card (This will end your turn)\n>",p[current_player].player_num);
            scanf("%d",&input);
            retry_option=true;
            result=0;

            //PLAY ONE CARD
            if (input==1) { 
                play_input_1=0;
                play_input_2=0;
                play_input_3=0;
                while (play_input_1<1 || play_input_1>p[current_player].hand_count) { //Does the card exist?
                    printf("Play which card? >");
                    scanf("%d",&play_input_1); 
                }

                while (play_input_3<1 || play_input_3>2) { //Did the user answer with one of the two piles?
                    printf("On which center card? >");
                    scanf("%d",&play_input_3);
                }

                if (play_input_3==1) { result=testOneCard(&p[current_player],play_input_1,deck.pile_1,deck.pile_1_wild,deck.pile_1_rainbow); }
                else if (play_input_3==2) { result=testOneCard(&p[current_player],play_input_1,deck.pile_2,deck.pile_1_wild,deck.pile_2_rainbow); }
                //printf("Test result: %d\n",result); 

                if (result>0) { //Play the card
                    discardCard(&p[current_player].hand_head,&p[current_player].hand_count,play_input_1,&deck.discard_head,&deck.discard_count,true); 
                    //Discard the card used

                    if (result==1) { //Number Match
                        printf("Card played. Player %d now has %d card(s).\n\n",p[current_player].player_num,p[current_player].hand_count);
                        drawNextPile(&deck,play_input_3);
                        //Discard the pile card and draw a new one
                    }
                    else if (result==2) { //Color Match
                        printf("Color bonus! Choose another card to replace the center card!\n");
                        displayCards(p[current_player].hand_head,p[current_player].hand_count,0);
                        play_input_1=0;
                        while (play_input_1<1 || play_input_1>p[current_player].hand_count) { 
                            printf (">");
                            scanf("%d",&play_input_1);
                        }
                        playToPile(&deck,play_input_3,&p[current_player],play_input_1); //Put the bonus card as the new pile card
                        //discardCard(&p[current_player].hand_head,&p[current_player].hand_count,play_input_3,&deck.discard_head,&deck.discard_count); 
                        printf("Player %d now has %d card(s).\n\n",p[current_player].player_num,p[current_player].hand_count);
                        
                    } 
                }
                else { printf("Cards don't match. Please try again.\n\n"); }
            }

            //PLAY TWO CARDS
            else if (input==2) { 
                if ( p[current_player].hand_count<2 ) { printf("You need two cards.\n"); }
                else {
                    play_input_1=0;
                    play_input_2=0;
                    play_input_3=0;
                    while (play_input_1<1 || play_input_1>p[current_player].hand_count) { //Does the card exist?
                        printf("First card to play? >");
                        scanf("%d",&play_input_1); 
                    }
                    while (play_input_2<1 || play_input_2>p[current_player].hand_count || play_input_1==play_input_2) { //Does the card exist and isn't the same card?
                        printf("Second card to play? >");
                        scanf("%d",&play_input_2); 
                    }
                    
                    while (play_input_3<1 || play_input_3>2) { //Did the user answer with one of the two piles?
                        printf("On which center card? >");
                        scanf("%d",&play_input_3);
                    }

                    if (play_input_3==1) { result=testTwoCard(&p[current_player],play_input_1,play_input_2,deck.pile_1,deck.pile_1_wild,deck.pile_1_rainbow); }
                    else if (play_input_3==2) { result=testTwoCard(&p[current_player],play_input_1,play_input_2,deck.pile_2,deck.pile_2_wild,deck.pile_2_rainbow); }
                    //printf("Test result: %d\n",result); 

                    if (result>0) { //Play the cards
                        discardCard(&p[current_player].hand_head,&p[current_player].hand_count,play_input_1,&deck.discard_head,&deck.discard_count,true); 
                        if (play_input_2>play_input_1) {
                            discardCard(&p[current_player].hand_head,&p[current_player].hand_count,play_input_2-1,&deck.discard_head,&deck.discard_count,true); 
                        }
                        else {
                            discardCard(&p[current_player].hand_head,&p[current_player].hand_count,play_input_2,&deck.discard_head,&deck.discard_count,true);
                        }
                        //Discard the cards used

                        if (result==1) { //Number Match
                            printf("Cards played. Player %d now has %d card(s).\n\n",p[current_player].player_num,p[current_player].hand_count);
                            drawNextPile(&deck,play_input_3);
                            //Discard the pile card and draw a new one
                        }
                        else if (result==2) { //Color Match
                            printf("Color bonus! Choose another card to replace the center card!\n");
                            displayCards(p[current_player].hand_head,p[current_player].hand_count,0);
                            play_input_1=0;
                            while (play_input_1<1 || play_input_1>p[current_player].hand_count) { 
                                printf (">");
                                scanf("%d",&play_input_1);
                            }
                            playToPile(&deck,play_input_3,&p[current_player],play_input_1); //Put the bonus card as the new pile card
                            //discardCard(&p[current_player].hand_head,&p[current_player].hand_count,play_input_3,&deck.discard_head,&deck.discard_count); 
                            printf("Player %d now has %d card(s).\n\n",p[current_player].player_num,p[current_player].hand_count);
                        }
                        else if (result==3) { //DOUBLE Color Match
                            printf("DOUBLE Color bonus! Choose another card to replace the center card!\n");
                            displayCards(p[current_player].hand_head,p[current_player].hand_count,0);
                            play_input_1=0;
                            while (play_input_1<1 || play_input_1>p[current_player].hand_count) { 
                                printf (">");
                                scanf("%d",&play_input_1);
                            }
                            playToPile(&deck,play_input_3,&p[current_player],play_input_1); //Put the bonus card as the new pile card
                            //discardCard(&p[current_player].hand_head,&p[current_player].hand_count,play_input_3,&deck.discard_head,&deck.discard_count); 
                            printf("Player %d now has %d card(s). All other players now must draw one card.\n\n",p[current_player].player_num,p[current_player].hand_count);
                            for (i=0;i<max_players;i++) { //Give everyone else one card
                                if (i==current_player) { continue; }
                                drawCard(&deck,&p[i]);
                            }
                        }
                    }
                    else { printf("Cards don't match. Please try again.\n\n"); }
                }
            }

            //DRAW CARD
            else if (input==3) { 
                play_input_1=0;
                play_input_2=0;
                printf("Drawing a card. Player %d now has %d card(s).\n",p[current_player].player_num,p[current_player].hand_count+1);
                drawCard(&deck,&p[current_player]);
                displayCards(p[current_player].hand_head,p[current_player].hand_count,0);
                displayPiles(deck.pile_1,deck.pile_2,deck.pile_1_wild,deck.pile_2_wild,deck.pile_1_rainbow,deck.pile_2_rainbow); 
                while (play_input_1<1 || play_input_1>p[current_player].hand_count) { //Did the user answer with one of the two piles?
                    printf("Select a card to replace one of the center cards. >");
                    scanf("%d",&play_input_1);
                }
                while (play_input_2<1 || play_input_2>2) { //Did the user answer with one of the two piles?
                    printf("On which center card? >");
                    scanf("%d",&play_input_2);
                }
                playToPile(&deck,play_input_2,&p[current_player],play_input_1);
                turn_active=false;
            }
        }
        //Now switch to the next turn
        if (new_round==false) {
            current_player++;
            if (current_player>=max_players) { current_player=0; }
            turn++;
        }
        else { 
            if (p[current_player].points>=points_to_win) { //Did someone get 200 points to win the game?
                printf("Player %s has won the match!\n",p[current_player].player_num);
                game_active=false;
            }
            printf("\nPoint totals:\n");
            for (i=0;i<max_players;i++) { printf("Player %d: %d\n",p[i].player_num,p[i].points); }
        }
        //End turn!
    }

    return 0;
}

/* 
--== DECK MANAGEMENT ==--
*/

//==CREATEDECK==
//Create the deck of Dos cards
//108 cards: 4 suits consisting of: 3 1-5 cards, 2 6-10 cards, 2 wild cards
//Includes 12 rainbow cards of value 2 that replaces any suit 2's
void createDeck(dealer *deck) {
    int i1=0, i2=0, j=0, j_max=0, j_type=0;
    deck->draw_count=0;
    deck->discard_count=0;
    card *buffer=NULL;

    printf("Creating deck...");
    for (i1=1;i1<=4;i1++) { //Suits
        for (i2=1;i2<=11;i2++) { //Card types without rainbow cards
            if (i2==2) { continue; } //Skip 2
            if (i2>=1 && i2<=5 && i2!=2) {  //Face 1-5 WITHOUT 2
                j_max=3;
                j_type=1;
            }
            else if (i2>=6 && i2<=10) { //Face 6-10
                j_max=2;
                j_type=1;
            }
            else if (i2==11) { //Face wild
                j_max=2;
                j_type=2;
            }
            for (j=0;j<j_max;j++) { //Now create the card
                buffer=(card *)malloc(sizeof (card)); //Create new instance of card
                buffer->value=i2;
                buffer->suit=i1;
                buffer->type=j_type;
                if (deck->draw_count==0) { //Catch the very first card so it doesn't crash the program
                    buffer->pt=NULL;
                    deck->draw_head=buffer; //And make it the head
                }
                else {
                    buffer->pt=deck->draw_head; //Attached the previous top card to the new top card
                    deck->draw_head=buffer; //And make the new card the head
                }
                deck->draw_count++; 
            }
        }
        printf("%d...",deck->draw_count);
    }
    for (i1=0;i1<12;i1++) { //The 12 rainbow cards (value 2)
        buffer=(card *)malloc(sizeof (card)); //Create new instance of card
        buffer->value=2;
        buffer->suit=5; //Rainbow suit
        buffer->type=3;
        buffer->pt=deck->draw_head; //Attached the previous top card to the new top card
        deck->draw_head=buffer; //And make the new card the head
        deck->draw_count++; 
    }
    printf("Done. %d cards in the deck.\n", deck->draw_count);
    return; //We've now created a list of cards simimlar in order to a factory new deck
}

//==SHUFFLEDECK==
//Shuffle the order of the stack of cards
//What we're gonna do is create a new HEAD pointer, and randomly pluck cards from the old list to place in sequence in the new list
//before returning the pointer to the new list
//
//What isn't shown is that four different solutions were attempted just to make this shuffler work without crashing or looping
//--REMEMBER THAT IT WILL RETRUN THE NEW HEAD ADDRESS, CALL THIS FUNCTION AS A VARIABLE REASSIGNMENT AND NOT ALONE--
card *shuffleDeck(card *head, int size) {

    printf("Shuffling deck...");
    card *s_head=NULL; //Setup
    card *s_next=NULL;
    card *buffer=head;
    card *buffer_tail=NULL;
    int i=0, j=0, j_rng=0;

    for (i=0;i<size;i++) {
        buffer=head;
        buffer_tail=NULL;
        j_rng=rand()%(size-i); //Pick a random card from the list (accounting for the fact that the list will be shrinking)
        for (j=0;j<j_rng;j++) {
            buffer_tail=buffer; //Move the buffer to the selected card
            buffer=buffer->pt;
        }
        if (buffer_tail==NULL) { head=buffer->pt; } //Check in case the card selected is the old HEAD
        else { buffer_tail->pt=buffer->pt; }
        s_head=buffer; //Switching the pointers around and stiching the gap in the old list
        s_head->pt=s_next;
        s_next=s_head;
        //printf("%d-%d\n",i,j_rng);
        //displayCards(head,size-i);
    }
    //displayCards(s_head,i);
    printf("Done.\n");

    return s_head; //Return the address to the new shuffled deck
}

//==RESTOCKDECK==
//Take the entire discard pile and relink it to the draw deck, before shuffling the draw deck again
void restockDeck(dealer *deck) {
    if (deck->discard_count<1 || deck->discard_head==NULL) { return; } //Catch

    card *buffer=NULL;
    printf("Restocking the deck.\n");
    while (deck->discard_count>0) {
        buffer=deck->discard_head; //Mark card for move
        deck->discard_head=buffer->pt; //Move discard HEAD up
        buffer->pt=deck->draw_head; //Move card to draw deck
        deck->draw_head=buffer; //Set new draw HEAD
        deck->draw_count=deck->draw_count+1; //Update counters
        deck->discard_count=deck->discard_count-1;
    }
    deck->discard_head=NULL; //Clear the HEAD pointer
    deck->draw_head=shuffleDeck(deck->draw_head,deck->draw_count); //DON'T FORGET TO UPDATE THE HEAD POSITION!!!!!

    return;
}

//==DEALPILES==
//Draw two cards from the deck and put them in the two piles
void dealPiles(dealer *deck) {
    if (deck->pile_1!=NULL || deck->pile_2!=NULL) { return; } //Catch

    printf("Drawing the two center cards.\n");

    card *buffer=NULL;
    //Grab first card
    buffer=deck->draw_head; //Select card
    deck->draw_head=buffer->pt; //Set next card to be new HEAD
    buffer->pt=NULL; //Disconnect card
    deck->pile_1=buffer; //Set as pile 1

    //Grab second card
    buffer=deck->draw_head; //Select card
    deck->draw_head=buffer->pt; //Set next card to be new HEAD
    buffer->pt=NULL; //Disconnect card
    deck->pile_2=buffer; //Set as pile 1

    //Update counters
    deck->draw_count=deck->draw_count-2;

    return;
}

//==DEALPLAYERS==
//Draw seven cards from the deck and give them to the player object
void dealPlayer(dealer *deck, player *hand) {
    int i=0;
    card *buffer=deck->draw_head;
    card *buffer_next=NULL;

    printf("Dealing to Player %d...",hand->player_num);
    for (i=0;i<7;i++) {
        if (hand->hand_count>0) { buffer_next=hand->hand_head; }
        hand->hand_head=buffer; //Set card to top of hand
        deck->draw_head=buffer->pt; //Set next card to top of deck
        hand->hand_head->pt=buffer_next; //Link
        deck->draw_count=deck->draw_count-1; //Update counters
        hand->hand_count=hand->hand_count+1;
        buffer=deck->draw_head; //Draw next card
    }
    printf("Done.\n");

    return;
}

//==CLEARHANDS==
//Discards all the cards in every player's hand. Simple enough.
void clearHands(player p[], int max_players, dealer *deck) {
    int i=0, j=0;
    for (i=0;i<max_players;i++) {
        for (j=0;j<p[i].hand_count;j++) {
            discardCard(&p[i].hand_head,&p[i].hand_count,1,&deck->discard_head,&deck->discard_count,false);
        }
    }
    return;
}

//==CLEARPILES==
//Discards all the pile cards. Simple enough.
void clearPiles(dealer *deck) {
    card *buffer=deck->discard_head;
    deck->discard_head=deck->pile_1; //Clear left card
    deck->discard_head->pt=buffer;
    deck->discard_head=deck->pile_2; //Clear right card
    deck->discard_head->pt=buffer;
    deck->discard_count=deck->discard_count+2;
    deck->pile_1=NULL;
    deck->pile_2=NULL;
    deck->pile_1_rainbow=0;
    deck->pile_1_wild=0;
    deck->pile_2_rainbow=0;
    deck->pile_2_wild=0;
    return;
}

/* 
--== GAME DISPLAY ==--
*/

//==DISPLAYCARDS==
//Display the cards in a numbered row (in full ASCII art!)
//This is done in rows of 15 cards each, using recursion and an iteration counter to make sure the card numbers maintain contiunitity
// ----
// |  |
// |  |
// ----
void displayCards(card *head, int size, int iteration) {
    //printf("\n%d\n",*size);
    int i=0;
    card *buffer=head; //Pointer to display cards

    if (buffer==NULL || size<1) { return; } //Catch

    for (i=0;i<size;i++) { //Row 0 (Indexes)
        printf(" %2.d  ",((iteration*15)+(i+1)));
    }
    printf("\n");

    for (i=0;i<size;i++) { //Row 1
        printf("---- ");
        //buffer=buffer->pt; //Next card
    }
    printf("\n");
    buffer=head; //Reset buffer
    for (i=0;i<size && i<15;i++) { //Row 2
        if (buffer->suit==1) { printf("| R| "); } //Red
        else if (buffer->suit==2) { printf("| B| "); } //Blue
        else if (buffer->suit==3) { printf("| G| "); } //Green
        else if (buffer->suit==4) { printf("| Y| "); } //Yellow
        else if (buffer->suit==5) { printf("| +| "); } //Rainbow
        else { printf("|  | "); }
        buffer=buffer->pt; //Next card
    }
    printf("\n");
    buffer=head; //Reset buffer
    for (i=0;i<size;i++) { //Row 3
        if (buffer->value<10 && buffer->value>0) { printf("| %d| ",buffer->value); } //Face numbers
        else if (buffer->value==10) { printf("|%d| ",buffer->value); }
        else if (buffer->value==11) { printf("| #| "); } //Wild
        else { printf("|  | "); }
        buffer=buffer->pt; //Next card
    }
    printf("\n");
    buffer=head; //Reset buffer
    for (i=0;i<size;i++) { //Row 4
        printf("---- ");
        //buffer=buffer->pt; //Next card
    }
    printf("\n\n");

    if (size>15) { //More than 15 cards, start new row of cards
        displayCards(buffer,(size-15),iteration+1);
    }
    return;
}

//==DISPLAYPILES==
//Display the two active piles (don't call if there aren't two active piles)
void displayPiles(card *left, card *right, int wild_1, int wild_2, int rainbow_1, int rainbow_2) {
    //Row 1
    printf("  Center\n");
    printf("  1     2\n");
    printf("----   ----\n");

    //Row 2 Left
    if (left->suit==1) { printf("| R|   "); } //Red
    else if (left->suit==2) { printf("| B|   "); } //Blue
    else if (left->suit==3) { printf("| G|   "); } //Green
    else if (left->suit==4) { printf("| Y|   "); } //Yellow
    else if (left->suit==5) { //Rainbow, handled under special conditions
        if (rainbow_1==1) { printf("| R|   "); } //Red
        else if (rainbow_1==2) { printf("| B|   "); } //Blue
        else if (rainbow_1==3) { printf("| G|   "); } //Green
        else if (rainbow_1==4) { printf("| Y|   "); } //Yellow
    } 
    else { printf("|  |   "); }
    //Row 2 Right
    if (right->suit==1) { printf("| R|   "); } //Red
    else if (right->suit==2) { printf("| B|   "); } //Blue
    else if (right->suit==3) { printf("| G|   "); } //Green
    else if (right->suit==4) { printf("| Y|   "); } //Yellow
    else if (right->suit==5) { //Rainbow, handled under special conditions
        if (rainbow_2==1) { printf("| R|   "); } //Red
        else if (rainbow_2==2) { printf("| B|   "); } //Blue
        else if (rainbow_2==3) { printf("| G|   "); } //Green
        else if (rainbow_2==4) { printf("| Y|   "); } //Yellow
    } 
    else { printf("|  |   "); }
    printf("\n");

    //Row 3 Left
    if (left->value<10 && left->value>0) { printf("| %d|   ",left->value); } //Face numbers
    else if (left->value==10) { printf("|%d|   ",left->value); }
    else if (left->value==11) { //Wild, handled under special conditions
        if (wild_1<10) { printf("| %d|   ",wild_1); }
        else { printf("|%d|   ",wild_1); }
    }
    else { printf("|  |  "); }
    //Row 3 Right
    if (right->value<10 && right->value>0) { printf("| %d|   ",right->value); } //Face numbers
    else if (right->value==10) { printf("|%d|   ",right->value); }
    else if (right->value==11) { //Wild, handled under special conditions
        if (wild_1<10) { printf("| %d|   ",wild_2); }
        else { printf("|%d|   ",wild_2); }
    }
    else { printf("|  |   "); }
    printf("\n");

    //Row 4
    printf("----   ----\n\n");

    return;
}

/* 
--== GAME FUNCTIONS ==--
*/

//==DISCARDCARD==
//We're given a card and its location to be moved to the discard pile
//Both HEADs are double pointers so any address changes are persistent
void discardCard(card **hand_head, int *hand_size, int hand_index, card **discard_head, int *discard_size, bool play) {
    if (*hand_size<1 || hand_index>*hand_size) { return; } //Catch
    //printf("discarding\n");
    card *buffer=*hand_head;
    card *buffer_tail=NULL;
    card *buffer_next=NULL;
    int i=0;

    for (i=1;i<*hand_size && i<hand_index;i++) { //Move buffer to selected card
        buffer_tail=buffer;
        buffer=buffer->pt;
    }
    buffer_next=buffer->pt;
    
    //Move the card
    if (buffer_tail==NULL) { *hand_head=buffer->pt; } //Check in case the card to be discarded is the HEAD
    else { buffer_tail->pt=buffer->pt; }
    buffer->pt=*discard_head; //Add to discard stack
    *discard_head=buffer;
    *hand_size=*hand_size-1;
    *discard_size=*discard_size+1;

    if (*hand_size==2 && play==true) { printf("D O S!\n"); } //Callout

    return;
}

//==DRAWCARD==
//Gives one card to the specific player
//First it needs to check if there are cards to draw, if not then it needs to perform the restock and shuffle
void drawCard(dealer *deck, player *hand) {
    if (deck->draw_count<2) { restockDeck(deck); };
    
    card *buffer=hand->hand_head;
    card *buffer_next=deck->draw_head->pt;

    hand->hand_head=deck->draw_head; //Move top card into hand
    deck->draw_head=buffer_next; //Set new deck HEAD to the following card
    hand->hand_head->pt=buffer; //Set next card from hand HEAD to disconnected card
    deck->draw_count=deck->draw_count-1; //Update counters
    hand->hand_count=hand->hand_count+1;

    return;
}

//==DRAWNEXTPILE==
//Discards the tagged pile card and draws a new one. If it's wild, prompts the player for either the suit or the value.
void drawNextPile(dealer *deck, int tag) {
    if (deck->draw_count<2) { restockDeck(deck); }; //Check if the deck needs to be restocked

    card *buffer=NULL;
    int input=0;

    if (tag==1) { //Replace left pile
        //DISCARD
        buffer=deck->pile_1; //Mark card
        buffer->pt=deck->discard_head; //Move to discard deck
        deck->discard_head=buffer; //Update HEAD
        deck->discard_count=deck->discard_count+1;
        deck->pile_1=NULL;
        deck->pile_1_rainbow=0;
        deck->pile_1_wild=0;
        //DRAW
        deck->pile_1=deck->draw_head; //Move top of draw deck
        deck->draw_head=deck->pile_1->pt; //Update HEAD
        deck->pile_1->pt=NULL;
        deck->draw_count=deck->draw_count-1;
        //Is it a wild/rainbow card?
        if (deck->pile_1->suit==5) { //Rainbow
            printf("Rainbow 2 card!\nWhat suit should it be?\n(1-Red 2-Blue 3-Green 4-Yellow)\n");
            while (input<1 || input>4) {
                printf(">");
                scanf("%d",&input);
            }
            deck->pile_1_rainbow=input;
        }
        if (deck->pile_1->value==11) { //Wild
            printf("Wild ");
            if (deck->pile_1->suit==1) { printf("Red "); }
            else if (deck->pile_1->suit==2) { printf("Blue "); }
            else if (deck->pile_1->suit==3) { printf("Green "); }
            else if (deck->pile_1->suit==4) { printf("Yellow "); }
            printf("card!\n");
            while (input<1 || input>10) {
                printf("What should its value be? (1-10) >");
                scanf("%d",&input);
            }
            deck->pile_1_wild=input;
        }
    }

    else if (tag==2) { //Replace right pile
        //DISCARD
        buffer=deck->pile_2; //Mark card
        buffer->pt=deck->discard_head; //Move to discard deck
        deck->discard_head=buffer; //Update HEAD
        deck->discard_count=deck->discard_count+1;
        deck->pile_2=NULL;
        deck->pile_2_rainbow=0;
        deck->pile_2_wild=0;
        //DRAW
        deck->pile_2=deck->draw_head; //Move top of draw deck
        deck->draw_head=deck->pile_2->pt; //Update HEAD
        deck->pile_2->pt=NULL;
        deck->draw_count=deck->draw_count-1;
        //Is it a wild/rainbow card?
        if (deck->pile_2->suit==5) { //Rainbow
            printf("Rainbow 2 card!\nWhat suit should it be?\n(1-Red 2-Blue 3-Green 4-Yellow)\n>");
            while (input<1 || input>4) {
                printf(">");
                scanf("%d",&input);
            }
            deck->pile_2_rainbow=input;
        }
        if (deck->pile_2->value==11) { //Wild
            printf("Wild ");
            if (deck->pile_2->suit==1) { printf("Red "); }
            else if (deck->pile_2->suit==2) { printf("Blue "); }
            else if (deck->pile_2->suit==3) { printf("Green "); }
            else if (deck->pile_2->suit==4) { printf("Yellow "); }
            printf("card!\n");
            while (input<1 || input>10) {
                printf("What should its value be? (1-10) >");
                scanf("%d",&input);
            }
            deck->pile_2_wild=input;
        }
    }

    return;
}

//==PLAYTOPILE==
//Takes the card input and puts it in the selected pile
void playToPile(dealer *deck, int tag, player *p, int hand_index) {

    if (p->hand_count<1 || hand_index>p->hand_count) { return; } //Catch

    card *buffer=p->hand_head;
    card *buffer_tail=NULL;
    card *buffer_next=NULL;
    card *buffer_2=NULL;
    int i=0, input=0;

    for (i=1;i<p->hand_count && i<hand_index;i++) { //Move buffer to selected card
        buffer_tail=buffer;
        buffer=buffer->pt;
    }
    buffer_next=buffer->pt;

    if (tag==1) { //Replace left pile
        //DISCARD
        buffer_2=deck->pile_1; //Mark card
        buffer_2->pt=deck->discard_head; //Move to discard deck
        deck->discard_head=buffer_2; //Update HEAD
        deck->discard_count=deck->discard_count+1;
        deck->pile_1=NULL;
        deck->pile_1_rainbow=0;
        deck->pile_1_wild=0;
        //REPLACE
        if (buffer_tail==NULL) { p->hand_head=buffer->pt; } //Check in case the card to be discarded is the HEAD
        else { buffer_tail->pt=buffer->pt; }//Replace links
        deck->pile_1=buffer;
        deck->pile_1->pt=NULL;
        p->hand_count=p->hand_count-1;
        //Is it a wild/rainbow card?
        if (deck->pile_1->suit==5) { //Rainbow
            printf("Rainbow 2 card!\nWhat suit should it be?\n(1-Red 2-Blue 3-Green 4-Yellow)\n>");
            while (input<1 || input>4) {
                printf(">");
                scanf("%d",&input);
            }
            deck->pile_1_rainbow=input;
        }
        if (deck->pile_1->value==11) { //Wild
            printf("Wild ");
            if (deck->pile_1->suit==1) { printf("Red "); }
            else if (deck->pile_1->suit==2) { printf("Blue "); }
            else if (deck->pile_1->suit==3) { printf("Green "); }
            else if (deck->pile_1->suit==4) { printf("Yellow "); }
            printf("card!\n");
            while (input<1 || input>10) {
                printf("What should its value be? (1-10) >");
                scanf("%d",&input);
            }
            deck->pile_1_wild=input;
        }
    }

    else if (tag==2) { //Replace right pile
        //DISCARD
        buffer_2=deck->pile_2; //Mark card
        buffer_2->pt=deck->discard_head; //Move to discard deck
        deck->discard_head=buffer_2; //Update HEAD
        deck->discard_count=deck->discard_count+1;
        deck->pile_2=NULL;
        deck->pile_2_rainbow=0;
        deck->pile_2_wild=0;
        //REPLACE
        if (buffer_tail==NULL) { p->hand_head=buffer->pt; } //Check in case the card to be discarded is the HEAD
        else { buffer_tail->pt=buffer->pt; }//Replace links
        deck->pile_2=buffer;
        deck->pile_2->pt=NULL;
        p->hand_count=p->hand_count-1;
        if (deck->pile_2->suit==5) { //Rainbow
            printf("Rainbow 2 card!\nWhat suit should it be?\n(1-Red 2-Blue 3-Green 4-Yellow)\n>");
            while (input<1 || input>4) {
                printf(">");
                scanf("%d",&input);
            }
            deck->pile_2_rainbow=input;
        }
        if (deck->pile_2->value==11) { //Wild
            printf("Wild ");
            if (deck->pile_2->suit==1) { printf("Red "); }
            else if (deck->pile_2->suit==2) { printf("Blue "); }
            else if (deck->pile_2->suit==3) { printf("Green "); }
            else if (deck->pile_2->suit==4) { printf("Yellow "); }
            printf("card!\n");
            while (input<1 || input>10) {
                printf("What should its value be? (1-10) >");
                scanf("%d",&input);
            }
            deck->pile_2_wild=input;
        }
    }

    if (p->hand_count==2) { printf("D O S!\n"); } //Callout

    return;
}

//==TESTONECARD==
//Checks the selected card aganist the piles
//0-No Match 1-Match 2-Color Match
int testOneCard(player *p, int index, card *pile, int wild, int rainbow) {
    card *buffer=p->hand_head;
    int i=0;

    for (i=1;i<p->hand_count && i<index;i++) { //Move buffer to selected card
        buffer=buffer->pt;
    }
    
    //Test aganist pile
    if (buffer->value==pile->value || (buffer->value==wild && pile->value==11) || buffer->value==11) { 
    //Match
        if (buffer->suit==pile->suit || (pile->suit==5 && buffer->suit==rainbow)) { return 2; } //Color match
        else { return 1; }
    }

    return 0;
}

//==TESTTWOCARD==
//Checks the selected cards aganist the pile
//0-No Match 1-Match 2-Color Match on one card 3-Color Match on both cards
int testTwoCard(player *p, int index_1, int index_2, card *pile, int wild, int rainbow) {
    card *buffer_1=p->hand_head;
    card *buffer_2=p->hand_head;

    int i=0;

    for (i=1;i<p->hand_count && i<index_1;i++) { //Move buffer to selected card
        buffer_1=buffer_1->pt;
    }
    for (i=1;i<p->hand_count && i<index_2;i++) { //Move buffer to selected card
        buffer_2=buffer_2->pt;
    }

    int total=buffer_1->value+buffer_2->value;
    if ((buffer_1->value==11 && buffer_2->value<pile->value) || (buffer_2->value==11 && buffer_1->value<pile->value)) { //Played a wild card
        total=pile->value;
    }
    
    //Test aganist pile
    if (total==pile->value || (total==wild && pile->value==11)) { //Match
        if (buffer_1->suit==pile->suit || (pile->suit==5 && buffer_1->suit==rainbow) || (buffer_1->suit==5)) { //Color match on card 1
            if (buffer_2->suit==pile->suit || (pile->suit==5 && buffer_2->suit==rainbow) || (buffer_2->suit==5)) { return 3; } //Double Color match
            return 2;
        }
        else if (buffer_2->suit==pile->suit || (pile->suit==5 && buffer_2->suit==rainbow) || (buffer_2->suit==5)) { return 2; } //Color match on card 2
        else { return 1; }
    }

    return 0;
}

//==AWARDPOINTS==
//End of round function to total up the card points from the other players
int awardPoints(player p[], int winner, int max_players) {
    int i=0, j=0, total=0;
    card *buffer=NULL;
    for (i=0;i<max_players;i++) {
        buffer=p[i].hand_head;
        if (i==winner) { continue; }
        for (j=0;j<p[i].hand_count;j++) {
            if (buffer->value<11 && buffer->value!=2) { total=total+buffer->value; } //Face value points
            else if (buffer->value==11) { total=total+40; } //40 pts for wilds
            else if (buffer->value==2) { total=total+20; } //20 pts for rainbows
            buffer=buffer->pt;
        }
    }
    return total;
}