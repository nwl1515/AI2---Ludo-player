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
    fann_type learning_rate;
    fann_type exploring_rate;
    fann_type discount_factor;
    struct fann *q_approximator;

    fann_type calc_reward();			///
    fann_type is_home(int pos);  // bool		///
    fann_type safe_now(int pos);			///
    fann_type safe_next(int pos);			///
    fann_type kill_next(int pos);			///
    fann_type defender_now(int pos); //bool	///
    fann_type defender_next(int pos); //bool	///
    fann_type finish_area(int pos); //bool		///
    fann_type distance_to_finish(int pos);		///

    std::vector<fann_type> feature_values(int pos);	///
    std::vector<fann_type> selected_feature_values;	
    fann_type selected_qvalue;
    fann_type approximate_qvalue(int pos);			///
    std::vector<int> possible_actions();		///
    int find_best_action();				///

    unsigned int num_train_data = 0;
    fann_type ** input_data;
    fann_type ** output_data;


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
