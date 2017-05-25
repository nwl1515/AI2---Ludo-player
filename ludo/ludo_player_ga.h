#ifndef LUDO_PLAYER_GA
#define LUDO_PLAYER_GA
#include <QObject>
#include <string>
#include <iostream>
#include "positions_and_dice.h"
#include "fann.h"
#include "floatfann.h"
#include <algorithm>
#include <random>
#include <fstream>

#define DEBUG false
#define CHROMOSONE_SIZE 61
#define INPUT_SIZE	8
#define POPULATION_SIZE 25
#define PARENTS 8
#define CHILDREN 16
#define SURVIVORS (POPULATION_SIZE-CHILDREN)

struct individual
{
	fann_type weights[CHROMOSONE_SIZE];
	fann_type fitness;
	fann_type best_fitness = 0;

};


class ludo_player_ga : public QObject {
    Q_OBJECT
public:
    std::vector<int> pos_start_of_turn;
    std::vector<int> pos_start_of_turn_orig;
    std::vector<int> pos_end_of_turn;
    int dice_roll;
    int make_decision_ga();
    fann_type crossover_rate = 0.7;
    fann_type mutation_rate = 0.20;

    struct fann *nn;
	
	fann_type random();

	// General representation
	fann_type num_home();
	fann_type num_finished();
	fann_type num_enemies();
	
	// Specific for each piece
	fann_type is_home(int a_pos);
	fann_type is_finished(int a_pos);
	fann_type safe_now(int a_pos);			//
	fann_type defender_now(int a_pos, int pos, std::vector<int> current_pos);
	fann_type finish_area(int a_pos);
	fann_type distance_to_finish_now(int a_pos);
	fann_type distance_to_enemy_behind(int a_pos);
	fann_type distance_to_enemy_front(int a_pos);
	fann_type last_part(int a_pos);

	fann_type active_weights[CHROMOSONE_SIZE];

	fann_type hit_star(int a_pos);
	void extra_chance();


	void init_pool();
	std::vector<individual> population;

	void set_individual(int i);

	static bool sorting_func(individual i, individual j);
	void sort_individuals();
	std::vector<individual> rank_selection();
	void new_generation();
	std::vector<individual> cross_over(std::vector<individual> parents);

	std::vector<int> ranking;

	//fann_type r_weights[17];


 
   	std::vector<fann_type> feature_values(int pos);
	void move_piece(int pos, bool &kill);


    	fann_type nn_value(std::vector<fann_type> inputs);
    	std::vector<int> possible_actions();
    	int find_best_action(std::vector<int> actions, fann_type max_a0, fann_type max_a1, fann_type max_a2, fann_type max_a3);

	bool die_next(int a_pos);		
	bool kill_next(int a_pos);		
	int pos_next(int pos, int dice);
	int globe_pos(int a_pos);
	int friends_on_pos(int a_pos);
	int enemy_on_pos(int a_pos);

	fann_type calc_fitness();
	void update_scores();

	int decision;


	void print_player_pos();


public:
    void save_neural_network(std::string path);
    void load_neural_network(std::string path);
    void create_new_neural_network();

    bool training = false;
    int wins = 0;

	int finished = 0;
	int home = 0;
	int left = 0;

    
    
	    



public:
    ludo_player_ga();
    ~ludo_player_ga();
signals:
    void select_piece(int);
    void turn_complete(bool);
public slots:
    void start_turn(positions_and_dice relative);
    void post_game_analysis(std::vector<int> relative_pos);
};

#endif // LUDO_PLAYER_GA
