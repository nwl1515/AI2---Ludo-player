#include "ludo_player_qlearning.h"
#include <random>

ludo_player_qlearning::ludo_player_qlearning():
    pos_start_of_turn(16),
    pos_end_of_turn(16),
    dice_roll(0),
    selected_feature_values(8)
{
	input_data = new fann_type*[50000];
	output_data = new fann_type*[50000];

}

void ludo_player_qlearning::create_new_neural_network()
{
	const unsigned int num_input = 8;
     	const unsigned int num_output = 1;
  	const unsigned int num_layers = 3;
        const unsigned int num_neurons_hidden = 50;
	q_approximator = fann_create_standard(num_layers, num_input, num_neurons_hidden, num_output);
        
   	fann_set_activation_function_hidden(q_approximator, FANN_SIGMOID);	// try with FANN_SIGMOID_SYMMETRIC (tanh)
	fann_set_activation_function_output(q_approximator, FANN_SIGMOID);	// try with something linear

}


ludo_player_qlearning::~ludo_player_qlearning()
{
	fann_destroy(q_approximator);

}

void ludo_player_qlearning::save_neural_network(std::string path)
{
	fann_save(q_approximator, path.c_str());

}

void ludo_player_qlearning::load_neural_network(std::string path)
{
	q_approximator = fann_create_from_file(path.c_str());

}

void ludo_player_qlearning::train_neural_network()
{
	const unsigned int num_inputs = 8;
	const unsigned int num_outputs = 1;

	// test training
	input_data[0] = new fann_type[8] {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
	input_data[1] = new fann_type[8] {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};
	output_data[0] = new fann_type[1] {1.0};
	output_data[1] = new fann_type[1] {0.0};

	struct fann_train_data *d = new struct fann_train_data;
	d->num_data = 2;
	d->num_input = 8;
	d->num_output = 1;
	d->input = input_data;
	d->output = output_data;

	fann_set_training_algorithm(q_approximator, FANN_TRAIN_BATCH);

	fann_train_on_data(q_approximator, d, 500000, 1000, 0.0000001);
	
	fann_type *calc_out;
	fann_type input[8] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};
	calc_out = fann_run(q_approximator, input);
	std::cout << "Test 1: " << calc_out[0] << std::endl;


	fann_type input2[8] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
	calc_out = fann_run(q_approximator, input2);
	std::cout << "Test 2: " << calc_out[0] << std::endl;


	fann_type input3[8] = {1.0,1.0,1.0,1.0,0.0,0.0,0.0,0.0};
	calc_out = fann_run(q_approximator, input3);
	std::cout << "Test 3: " << calc_out[0] << std::endl;

	std::cout << fann_test_data(q_approximator, d) <<  std::endl;
	

	
}

int ludo_player_qlearning::make_decision(){
    if(dice_roll == 6){
        for(int i = 0; i < 4; ++i){
            if(pos_start_of_turn[i]<0){
                return i;
            }
        }
        for(int i = 0; i < 4; ++i){
            if(pos_start_of_turn[i]>=0 && pos_start_of_turn[i] != 99){
                return i;
            }
        }
    } else {
        for(int i = 0; i < 4; ++i){
            if(pos_start_of_turn[i]>=0 && pos_start_of_turn[i] != 99){
                return i;
            }
        }
        for(int i = 0; i < 4; ++i){ //maybe they are all locked in
            if(pos_start_of_turn[i]<0){
                return i;
            }
        }
    }
    return -1;
}

void ludo_player_qlearning::start_turn(positions_and_dice relative){
    pos_start_of_turn = relative.pos;
    dice_roll = relative.dice;
    int decision = make_decision();
    emit select_piece(decision);
}

void ludo_player_qlearning::post_game_analysis(std::vector<int> relative_pos){
    pos_end_of_turn = relative_pos;
    bool game_complete = true;
    for(int i = 0; i < 4; ++i){
        if(pos_end_of_turn[i] < 99){
            game_complete = false;
        }
    }
    emit turn_complete(game_complete);
}
