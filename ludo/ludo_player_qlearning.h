#ifndef LUDO_PLAYER_QLEARNING_H
#define LUDO_PLAYER_QLEARNING_H
#include <QObject>
#include <string>
#include <iostream>
#include "positions_and_dice.h"
#include "doublefann.h"


class ludo_player_qlearning : public QObject {
    Q_OBJECT
private:
    std::vector<int> pos_start_of_turn;
    std::vector<int> pos_end_of_turn;
    int dice_roll;
    int make_decision();
	int make_decision_qlearning();
    fann_type learning_rate;
    fann_type exploring_rate;
    fann_type discount_factor;
    struct fann *q_approximator;

    fann_type calc_reward();			///


	// General representation
	fann_type num_home();
	fann_type num_finished();
	
	// Specific for each piece
    fann_type is_home(int a_pos);
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



    std::vector<fann_type> feature_values();
	std::vector<fann_type> feature_values_next(int pos);
    //std::vector<fann_type> feature_values(int pos);
    std::vector<fann_type> state; 
	std::vector<fann_type> old_state;
	std::vector<int> old_pos;
	fann_type reward = 0;	
	bool killed_player = false;

	
    fann_type old_qvalue;
    fann_type approximate_qvalue(int pos);			///
    std::vector<int> possible_actions();
    int find_best_action(std::vector<int> actions);				///

    bool die_next(int a_pos);		
    bool kill_next(int a_pos);		
    int pos_next(int pos, int dice);
    int globe_pos(int a_pos);
    int friends_on_pos(int a_pos);
    int enemy_on_pos(int a_pos);

    unsigned int num_train_data = 0;
    fann_type ** input_data;
    fann_type ** output_data;


    void print_player_pos();


public:
    void save_neural_network(std::string path);
    void load_neural_network(std::string path);
    void create_new_neural_network();
    void train_neural_network();
    
    
	    



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
