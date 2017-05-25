#include "dialog.h"
#include <QApplication>
#include "game.h"
#include <vector>
#include "ludo_player.h"
#include "ludo_player_random.h"
#include "ludo_player_ga.h"
#include "positions_and_dice.h"

Q_DECLARE_METATYPE( positions_and_dice )

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    qRegisterMetaType<positions_and_dice>();

    //instanciate the players here
    ludo_player p2;
    ludo_player p3, p4;
    ludo_player_ga p1;
   


    game g;
    g.setGameDelay(0); //if you want to see the game, set a delay

    /* Add a GUI <-- remove the '/' to uncomment block
    *Dialog w;
    QObject::connect(&g,SIGNAL(update_graphics(std::vector<int>)),&w,SLOT(update_graphics(std::vector<int>)));
    QObject::connect(&g,SIGNAL(set_color(int)),                   &w,SLOT(get_color(int)));
    QObject::connect(&g,SIGNAL(set_dice_result(int)),             &w,SLOT(get_dice_result(int)));
    QObject::connect(&g,SIGNAL(declare_winner(int)),              &w,SLOT(get_winner()));
    QObject::connect(&g,SIGNAL(close()),&a,SLOT(quit()));
    w.show();
    /*/ //Or don't add the GUI
    QObject::connect(&g,SIGNAL(close()),&a,SLOT(quit()));
    //*/

    //set up for each player
    QObject::connect(&g, SIGNAL(player1_start(positions_and_dice)),&p1,SLOT(start_turn(positions_and_dice)));
    QObject::connect(&p1,SIGNAL(select_piece(int)),                &g, SLOT(movePiece(int)));
    QObject::connect(&g, SIGNAL(player1_end(std::vector<int>)),    &p1,SLOT(post_game_analysis(std::vector<int>)));
    QObject::connect(&p1,SIGNAL(turn_complete(bool)),              &g, SLOT(turnComplete(bool)));

    QObject::connect(&g, SIGNAL(player2_start(positions_and_dice)),&p2,SLOT(start_turn(positions_and_dice)));
    QObject::connect(&p2,SIGNAL(select_piece(int)),                &g, SLOT(movePiece(int)));
    QObject::connect(&g, SIGNAL(player2_end(std::vector<int>)),    &p2,SLOT(post_game_analysis(std::vector<int>)));
    QObject::connect(&p2,SIGNAL(turn_complete(bool)),              &g, SLOT(turnComplete(bool)));

    QObject::connect(&g, SIGNAL(player3_start(positions_and_dice)),&p3,SLOT(start_turn(positions_and_dice)));
    QObject::connect(&p3,SIGNAL(select_piece(int)),                &g, SLOT(movePiece(int)));
    QObject::connect(&g, SIGNAL(player3_end(std::vector<int>)),    &p3,SLOT(post_game_analysis(std::vector<int>)));
    QObject::connect(&p3,SIGNAL(turn_complete(bool)),              &g, SLOT(turnComplete(bool)));

    QObject::connect(&g, SIGNAL(player4_start(positions_and_dice)),&p4,SLOT(start_turn(positions_and_dice)));
    QObject::connect(&p4,SIGNAL(select_piece(int)),                &g, SLOT(movePiece(int)));
    QObject::connect(&g, SIGNAL(player4_end(std::vector<int>)),    &p4,SLOT(post_game_analysis(std::vector<int>)));
    QObject::connect(&p4,SIGNAL(turn_complete(bool)),              &g, SLOT(turnComplete(bool)));


    p1.create_new_neural_network();
    p1.init_pool();

	 std::ofstream f("data2.txt");


// Run for 10 generations
for(int k = 0; k< 2000; k++) 
{

    // Find fitness values for each individual
    for(int i = 0; i< POPULATION_SIZE; i++)
    {
	p1.set_individual(i);
	p1.wins = 0;
	// Test each individual for 500 games
	for(int j = 0; j< 200; j++)
	{
		g.start();
		a.exec();
		while (a.closingDown())
		{
			std::cout << "Closing Down" << std::endl;
		}

		p1.update_scores();
		g.reset();
		if(g.wait()){}

	}
	//std::cout << "Num home: " << p1.home << std::endl;
	//std::cout << "Num Finished: " <<  p1.finished << std::endl;
	//std::cout << "Num won: " << p1.wins << std::endl;
        p1.population.at(i).fitness = p1.calc_fitness()/200.0;
    }	
	//p1.extra_chance();
	p1.sort_individuals();
	std::cout << "Best fitness after " << k << " Generations is " << std::endl;
	//for(int w = 0; w < p1.population.size(); w++) 
		std::cout << p1.population.at(0).fitness << std::endl;
	if(!f.fail())
	{
		f << k << '\t' << p1.population.at(0).fitness << std::endl;

	}
	std::cout << std::endl;
	p1.new_generation();
}		
	f.close();
	//p1.sort_individuals();
	//for(int i = 0; i< 25; i++)
	//	std::cout << "Fitness value for individual " << i << " = " << p1.population.at(i).fitness << std::endl;
	
	

	

	

	
    return 0;
}
