#ifndef LUDO_PLAYER_QLEARNING_H
#define LUDO_PLAYER_QLEARNING_H
#include <QObject>
#include <string>
#include <iostream>
#include "positions_and_dice.h"
#include "fann.h"

#define DEBUG false


class ludo_player_qlearning : public QObject {
    Q_OBJECT
public:
    std::vector<int> pos_start_of_turn;
    std::vector<int> pos_start_of_turn_orig;
    std::vector<int> pos_end_of_turn;
    int dice_roll;
    int make_decision_qlearning();
    fann_type learning_rate = 0.01;
    fann_type exploring_rate = 0.9;
    fann_type discount_factor = 0.9;
    struct fann *q_approximator;

    fann_type calc_reward(std::vector<fann_type> old_state, std::vector<fann_type> state, int old_pos, int new_pos, bool kill);		


	// General representation
	fann_type num_home();
	fann_type num_finished();
	
	// Specific for each piece
    fann_type is_home(int a_pos);
    fann_type is_finished(int a_pos);
    fann_type safe_now(int a_pos);			//
    fann_type defender_now(int a_pos, int pos, std::vector<int> current_pos);
    fann_type finish_area(int a_pos);
    fann_type distance_to_finish_now(int a_pos);
    fann_type distance_to_enemy_behind(int a_pos);
    fann_type distance_to_enemy_front(int a_pos);

	// not used
    fann_type safe_next(int pos);			//
    fann_type defender_next(int pos);	
    fann_type distance_to_finish_next(int pos);	

    std::vector<fann_type> current_state_a0; 
    std::vector<fann_type> current_state_a1; 
    std::vector<fann_type> current_state_a2; 
    std::vector<fann_type> current_state_a3; 

    std::vector<fann_type> next_state_a0;
    std::vector<fann_type> next_state_a1;
    std::vector<fann_type> next_state_a2;
    std::vector<fann_type> next_state_a3;


   bool winning;




    void move_piece(int pos, bool &kill);
    std::vector<fann_type> feature_values(int pos);

	bool win_game();


    fann_type q_approx(std::vector<fann_type> inputs);
    std::vector<int> possible_actions();
    int find_best_action(std::vector<int> actions, fann_type max_a0, fann_type max_a1, fann_type max_a2, fann_type max_a3);

    fann_type max_q(fann_type a0, fann_type a1, fann_type a2, fann_type a3);

    bool die_next(int a_pos);		
    bool kill_next(int a_pos);		
    int pos_next(int pos, int dice);
    int globe_pos(int a_pos);
    int friends_on_pos(int a_pos);
    int enemy_on_pos(int a_pos);

	int decision;


    struct fann_train_data *training_data;
    fann_type ** input_data;
    fann_type ** output_data;


    void print_player_pos();


public:
    void save_neural_network(std::string path);
    void load_neural_network(std::string path);
    void create_new_neural_network();
    void train_neural_network();

    bool training = false;
    int wins = 0;
    unsigned int num_train_data = 0;
    
    
	    



public:
    ludo_player_qlearning();
    ~ludo_player_qlearning();
signals:
    void select_piece(int);
    void turn_complete(bool);
public slots:
    void start_turn(positions_and_dice relative);
    void post_game_analysis(std::vector<int> relative_pos);
};

#endif // LUDO_PLAYER_QLEARNING_H
