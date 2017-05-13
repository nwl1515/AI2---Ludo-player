#include "ludo_player_qlearning.h"
#include <random>
#include <ctime>

ludo_player_qlearning::ludo_player_qlearning():
    pos_start_of_turn(16),
    pos_end_of_turn(16),
    dice_roll(0)
{
	input_data = new fann_type*[100000];
	output_data = new fann_type*[100000];

	old_state = std::vector<fann_type>(0);
	old_pos = std::vector<int>(0);
	old_pos.push_back(-1);
	old_pos.push_back(-1);
	old_pos.push_back(-1);
	old_pos.push_back(-1);
	state = std::vector<fann_type>(0);

	// initial state setup
	old_state.push_back(1);
	old_state.push_back(0);
	for(int i = 0; i< 4; i++)
	{
		old_state.push_back(1);
		old_state.push_back(1);
		old_state.push_back(0);
		old_state.push_back(1);
		old_state.push_back(1);
		old_state.push_back(1);

	}

	training_data = new struct fann_train_data;
	//training_data->num_data = 2;
	training_data->num_input = 26;
	training_data->num_output = 1;
	training_data->input = input_data;
	training_data->output = output_data;

	srand(time(NULL));


	
	for(int i = 0; i< 100000; i++)
	{
		input_data[i] = new fann_type[26];
		output_data[i] = new fann_type[1];
	}



}

void ludo_player_qlearning::reset_states()
{
	old_state = std::vector<fann_type>(0);
	old_pos = std::vector<int>(0);
	old_pos.push_back(-1);
	old_pos.push_back(-1);
	old_pos.push_back(-1);
	old_pos.push_back(-1);

	// initial state setup
	old_state.push_back(1);
	old_state.push_back(0);
	for(int i = 0; i< 4; i++)
	{
		old_state.push_back(1);
		old_state.push_back(1);
		old_state.push_back(0);
		old_state.push_back(1);
		old_state.push_back(1);
		old_state.push_back(1);

	}
	//num_train_data = 0;


}

void ludo_player_qlearning::create_new_neural_network()
{
	const unsigned int num_input = 26;
     	const unsigned int num_output = 1;
  	const unsigned int num_layers = 4;
        const unsigned int num_neurons_hidden = 50;
	q_approximator = fann_create_standard(num_layers, num_input, num_neurons_hidden, num_neurons_hidden, num_output);
        
   	fann_set_activation_function_hidden(q_approximator, FANN_SIGMOID);	// try with FANN_SIGMOID_SYMMETRIC (tanh)
	fann_set_activation_function_output(q_approximator, FANN_LINEAR);	// try with something linear

	fann_set_training_algorithm(q_approximator, FANN_TRAIN_BATCH);
	fann_set_learning_rate(q_approximator, learning_rate);

}


ludo_player_qlearning::~ludo_player_qlearning()
{
	print_player_pos();
	fann_destroy(q_approximator);
	for(int i = 0; i< 100000; i++)
	{
		delete[] input_data[i];
		delete[] output_data[i];
	}

	delete[] input_data;
	delete[] output_data;

}

std::vector<int> ludo_player_qlearning::possible_actions()
{
	std::vector<int> actions(0);
	for(int i = 0; i< 4; i++)
	{
		if(pos_start_of_turn[i] == -1 && dice_roll == 6)
			actions.push_back(i);
		else if(pos_start_of_turn[i] >= 0 && pos_start_of_turn[i] < 56)
			actions.push_back(i);

	}

	return actions;


}


fann_type ludo_player_qlearning::is_home(int a_pos)
{
	if(a_pos < 0)
		return 1.0;
	else
		return 0.0;

}

fann_type ludo_player_qlearning::finish_area(int a_pos)
{
	if(a_pos >= 51 && a_pos <= 55)
		return 1.0;
	else
		return 0.0;

}

fann_type ludo_player_qlearning::distance_to_finish_now(int a_pos)
{
	if(a_pos == 99)
		return 0.0;
	else if(a_pos == -1)
		return 1.0;
	else
		return ((fann_type)56 - (fann_type)a_pos)/((fann_type)56.0);


}



fann_type ludo_player_qlearning::defender_now(int a_pos, int pos, std::vector<int> current_pos)
{
	if(a_pos == -1 || a_pos >= 51 || a_pos == 0 || a_pos == 8 || a_pos == 21 || a_pos == 34 || a_pos == 47 || a_pos == 99)
	{
		return 0.0;
	}
	else
	{
		int defenders = 0;
		for(int i = 0; i< 4; i++)
		{	

			if(i != pos)
			{
				if(a_pos == current_pos[i])
					defenders++;

			}

		}
		if(defenders > 2)
			return 0.5;
		else if(defenders == 2)
			return 1.0;
		else
			return 0.0;

	}


}

int ludo_player_qlearning::pos_next(int pos, int dice)
{
	// Move to new position
	if(pos_start_of_turn[pos] == -1 && dice == 6)
		return 0;
	else if(pos_start_of_turn[pos] == -1)
		return -1;

	int next_pos = pos_start_of_turn[pos] + dice;

	// Account for stars
	if(next_pos == 5 || next_pos == 18 || next_pos == 31 || next_pos == 44) 
		next_pos += 6;
	else if(next_pos == 11 || next_pos == 24 || next_pos == 37)
		next_pos += 7;
	else if(next_pos == 50)
		return 99;
	
	if(next_pos == 56)
		return 99;
	
	else if(next_pos > 56)
		return (next_pos - (next_pos - 56)*2); 
	// if pos is 57 == finished
	else
		return next_pos;

}

int ludo_player_qlearning::globe_pos(int a_pos)
{
	// Check for enemy globes
	if(a_pos == 0 || a_pos == 8 || a_pos == 21 || a_pos == 34 || a_pos == 47)
		return 1;
	else if(a_pos == 13 || a_pos == 26 || a_pos == 39)
		return 2;
	else
		return 0;

}

int ludo_player_qlearning::friends_on_pos(int a_pos)
{
	int num = 0;
	for(int i = 0; i < 4; i++)
	{
		if(pos_start_of_turn[i] == a_pos)
			num++;

	}
	return num;

}


int ludo_player_qlearning::enemy_on_pos(int a_pos)
{
	int num = 0;
	for(int i = 4; i< 16; i++)
	{
		if(pos_start_of_turn[i] == a_pos)
			num++;

	}
	return num;

}


bool ludo_player_qlearning::die_next(int a_pos)
{
	if(a_pos <= 0)
		return false;
	else if(globe_pos(a_pos) && enemy_on_pos(a_pos))
		return true;
	else if(enemy_on_pos(a_pos) >= 2)
		return true;
	else
		return false; 

}

fann_type ludo_player_qlearning::defender_next(int pos)
{
	int next_pos = pos_next(pos, dice_roll);
	if(die_next(next_pos) || globe_pos(next_pos) || next_pos >= 51)
		return 0.0;
	else if(friends_on_pos(next_pos) == 2)
		return 1.0;
	else if(friends_on_pos(next_pos) >=3)
		return 0.5;
	else
		return 0.0;
	
}

fann_type ludo_player_qlearning::distance_to_finish_next(int pos)
{
	if(pos_next(pos, dice_roll) == 57 || pos_start_of_turn[pos] == 99)
		return 0.0;
	else if(pos_start_of_turn[pos] == -1 || die_next(pos_next(pos, dice_roll)))
		return 1.0;
	else
		return std::fabs((56.0 - (fann_type)pos_next(pos, dice_roll))/56.0);
		
}


bool ludo_player_qlearning::kill_next(int a_pos)
{

	// Kill this turn
	if(globe_pos(a_pos) == 0 && enemy_on_pos(a_pos) == 1)
		return true;
	else 		
		return false;

}

fann_type ludo_player_qlearning::safe_now(int a_pos)
{
	if(globe_pos(a_pos) == 1 || a_pos > 50 || a_pos == -1)
		return 1.0;
	else if(globe_pos(a_pos) == 2)
		return 0.5;
	else if(friends_on_pos(a_pos) >=2 && a_pos != -1)
		return 0.5;
	else
	{
		
		return 0.0;	// Check for enemies nearby?

	}


}


fann_type ludo_player_qlearning::safe_next(int pos)
{
	int a_pos = pos_next(pos, dice_roll);
	if(globe_pos(a_pos) == 1 || a_pos > 50)
		return 1.0;
	else if(globe_pos(a_pos) == 2)
		return 0.5;
	else if(friends_on_pos(a_pos) >=2 && a_pos != -1)
		return 0.5;
	else
	{
		
		return 0.0;	// Check for enemies nearby?

	}

}

fann_type ludo_player_qlearning::distance_to_enemy_behind(int a_pos)
{
	int nearest_distance = 50;
	for(int i = 4; i< 16; i++)
		if(a_pos > pos_start_of_turn[i] && (pos_start_of_turn[i] >= 0) && (a_pos >= 0) && (a_pos <= 50))
		{
			int distance = a_pos - pos_start_of_turn[i];
			if(distance < nearest_distance)
				nearest_distance = distance;

		}	


	return ((fann_type)nearest_distance)/50.0;
	

}



fann_type ludo_player_qlearning::distance_to_enemy_front(int a_pos)
{
	int nearest_distance = 50;
	for(int i = 4; i< 16; i++)
	{
		if((a_pos < pos_start_of_turn[i]) && (pos_start_of_turn[i] <= 50) && (a_pos >= 0) && (a_pos <= 50))
		{
			int distance = pos_start_of_turn[i] - a_pos;
			if(distance < nearest_distance)
				nearest_distance = distance;

		} 


	}
	
	return ((fann_type)nearest_distance)/50.0;


}

fann_type ludo_player_qlearning::num_home()
{
	int num = 0;
	for(int i = 0; i< 4; i++)
	{
		if(pos_start_of_turn[i] == -1)
			num++;

	}

	return num;

}

fann_type ludo_player_qlearning::num_finished()
{
	int num = 0;
	for(int i = 0; i<4; i++)
	{
		if(pos_start_of_turn[i] == 99)
			num++;

	}

	return num;


}
/*
std::vector<fann_type> ludo_player_qlearning::feature_values(int pos)
{
	std::vector<fann_type> res(7);
	res.at(0) = is_home(pos);
	res.at(1) = safe_now(pos);
	res.at(2) = defender_now(pos);
	res.at(3) = finish_area(pos);
	res.at(4) = distance_to_finish_now(pos);
	res.at(5) = distance_to_enemy_behind(pos);
	res.at(6) = distance_to_enemy_front(pos);

	return res;

}*/

std::vector<fann_type> ludo_player_qlearning::feature_values()
{
	std::vector<fann_type> res(0);
	res.push_back(0);//num_home()/4.0);
	res.push_back(0);//num_finished()/4.0);
	for(int i = 0; i< 4; i++)
	{
		int a_pos = pos_start_of_turn[i];
		res.push_back(0);//is_home(a_pos));
		res.push_back(0);//safe_now(a_pos));
		res.push_back(defender_now(a_pos, i, pos_start_of_turn));
		res.push_back(0);//distance_to_finish_now(a_pos));
		res.push_back(0);//distance_to_enemy_behind(a_pos));
		res.push_back(0);//distance_to_enemy_front(a_pos));


	}

	return res;


}

std::vector<fann_type> ludo_player_qlearning::feature_values_next(int pos)
{
	std::vector<int> temp_pos = pos_start_of_turn;
	std::vector<fann_type> new_features(0);
	int new_pos = pos_next(pos, dice_roll);
	if(die_next(new_pos))
	{
		pos_start_of_turn[pos] = -1;
	}
	else if(kill_next(new_pos))
	{
		for(int i = 4; i<16; i++)
		{
			if(pos_start_of_turn[i] == new_pos)
				pos_start_of_turn[i] = -1;
	
		}
		pos_start_of_turn[pos] = new_pos;

	}
	else
	{
		pos_start_of_turn[pos] = new_pos;
	}

	new_features = feature_values();

	pos_start_of_turn = temp_pos;

	return new_features;
	


}

void ludo_player_qlearning::print_player_pos()
{
	std::cout << "Player Positions: \n";
	for(int i = 0; i< 16; i++)
		std::cout << pos_start_of_turn[i] << " , ";

	std::cout << std::endl;


}



void ludo_player_qlearning::save_neural_network(std::string path)
{
	fann_save(q_approximator, path.c_str());

}

void ludo_player_qlearning::load_neural_network(std::string path)
{
	q_approximator = fann_create_from_file(path.c_str());
	fann_set_training_algorithm(q_approximator, FANN_TRAIN_BATCH);
   	fann_set_activation_function_hidden(q_approximator, FANN_SIGMOID_SYMMETRIC);	// try with FANN_SIGMOID_SYMMETRIC (tanh)
	fann_set_activation_function_output(q_approximator, FANN_SIGMOID_SYMMETRIC);	// try with something linear
	fann_set_learning_rate(q_approximator, learning_rate);


}

void ludo_player_qlearning::train_neural_network()
{

	/*// test training
	for(int i = 0; i< 26; i++)
	{
		input_data[0][i] = 0.0;
		input_data[1][i] = 1.0;
		input_data[2][i] = 0.5;
	}
	output_data[0][0] = 1.0;
	output_data[1][0] = 0.0;
	output_data[2][0] = 0.5;


	training_data->num_data = 3;

	fann_train_on_data(q_approximator, training_data, 500000, 1000, 0.0000001);
	
	fann_type *calc_out;
	fann_type input[26];
	for(int i = 0; i<26; i++)
	 	input[i] = 1.0;
	calc_out = fann_run(q_approximator, input);
	std::cout << "Test 1: " << calc_out[0] << std::endl;


	for(int i = 0; i<26; i++)
	 	input[i] = 0.0;
	calc_out = fann_run(q_approximator, input);
	std::cout << "Test 2: " << calc_out[0] << std::endl;


	for(int i = 0; i<26; i++)
	 	input[i] = 0.5;
	calc_out = fann_run(q_approximator, input);
	std::cout << "Test 3: " << calc_out[0] << std::endl;

	std::cout << fann_test_data(q_approximator, training_data) <<  std::endl;
*/	
	training_data->num_data = num_train_data;
	//fann_train_on_data(q_approximator, training_data, 10000, 0, 0.00001);
	fann_train_epoch(q_approximator, training_data);	
	num_train_data = 0;
	learning_rate = learning_rate*0.95;
	fann_set_learning_rate(q_approximator, learning_rate);
	

	
}

fann_type ludo_player_qlearning::calc_reward()
{
	int reward = 0;
	// win is not included here
	if(killed_player)
		reward += 1;

	for(int i = 0; i< 4; i++)
	{
		if((old_state[4] == 0 && state[4] >0) || (old_state[10] == 0 && state[10] >0) || (old_state[16] == 0 && state[16] >0) || (old_state[22] == 0 && state[22] >0))
		{
			// Defend
			reward += 0.5;
		}

		if((state[4] == 0 && old_state[4] >0) || (state[10] == 0 && old_state[10] >0) || (state[16] == 0 && old_state[16] >0) || (state[22] == 0 && old_state[22] >0))
		{
			//undefend
			reward -= 0.5;
		}

		if(old_pos[i] >= 0 && pos_start_of_turn[i] == -1)
		{
			//got killed
			reward -= 2;

		}

		if(old_pos[i] == -1 && pos_start_of_turn[i] >= 0)
		{
			// Move out
			reward += 2;
		}

		if(old_pos[i] != 99 && pos_start_of_turn[i] == 99)
		{
			// Finished one piece
			reward += 5;
		
		}

	}

	return reward;
}

int ludo_player_qlearning::find_best_action(std::vector<int> actions, fann_type &max)
{
	int action = 0;

	for(int i = 0; i< actions.size(); i++)
	{
		std::vector<fann_type> input_vec = feature_values_next(actions.at(i));

		fann_type q_value = q_approx(input_vec);
		std::cout << "Q_value for player: " << actions.at(i) << " is " << q_value << std::endl;
		if(q_value > max)
		{
			
			max = q_value;
			action = actions.at(i);
		}
		


	}

	return action;


}

fann_type ludo_player_qlearning::q_approx(std::vector<fann_type> inputs)
{
	fann_type input[inputs.size()];
	//std::cout << "Inputs size: " << inputs.size() << std::endl;
	for(int j = 0; j< inputs.size(); j++)
	{
		input[j] = inputs.at(j);

	}

	fann_type *q_value = fann_run(q_approximator, input);
	fann_type res = q_value[0];
	std::cout << "q_value: " << res << std::endl;
	//delete[] q_value;
	return res;

}	

int ludo_player_qlearning::make_decision_qlearning()
{

	state = feature_values();
	reward = calc_reward();
	std::vector<int> pos_actions = possible_actions();
	fann_type max = -1000000000;

	print_player_pos();
	//std::cout << "Feature values for Player 1:" << std::endl;
	//for(int i = 0; i< state.size(); i++)
	//	std::cout << state.at(i) << std::endl;
	//std::cout << std::endl;

	//std::cout << "Reward is: " << reward << std::endl;
	//std::cout << "Num Possible Action: " << pos_actions.size() << std::endl;

	int action = 0;
	if(pos_actions.size() >0)
	{
		action = find_best_action(pos_actions, max);

	
		
		if(training)
		{
			// General Q-learning rule
			//fann_type q_old = q_approx(state);
			//output_data[num_train_data][0] = q_old + learning_rate*(reward + discount_factor*max - q_old);


			// Pac-man variant?
		
			output_data[num_train_data][0] = reward + discount_factor*max;



			for(int i = 0; i< state.size(); i++)
			{
				input_data[num_train_data][i] = state.at(i);

			}
			num_train_data++;


			double exploration_activation = (double)(rand()%1000)/1000.0;
			if(exploration_activation < exploring_rate)
			{
				int randnum = rand()%(pos_actions.size());
				action = pos_actions.at(randnum);

			}
		}

		old_pos = pos_start_of_turn;
		old_state = state;



	}
	else
	{
		for(int i = 0; i< 4; i++)
		{
			if(pos_start_of_turn[i] == -1)
				action = i;
		}
	}

	std::cout << "Selected action: " << action << std::endl;
	std::cout << std::endl;
	return action;

}

int ludo_player_qlearning::make_decision(){
	print_player_pos();
	std::vector<fann_type> fv = feature_values();
	std::cout << "Feature values for Player 1:" << std::endl;
	for(int i = 0; i< fv.size(); i++)
		std::cout << fv.at(i) << std::endl;

	std::cout << std::endl;



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
    int decision = make_decision_qlearning();
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

	if(game_complete)
	{
		wins++;
		if(training)
		{
			// won = reward of 10 in winning state
			output_data[num_train_data][0] = 500;
			std::vector<int> temp_pos = pos_start_of_turn;
			pos_start_of_turn = pos_end_of_turn;
			state = feature_values();
			pos_start_of_turn = temp_pos;

			for(int i = 0; i< state.size(); i++)
			{
				input_data[num_train_data][i] = state.at(i);

			}
			num_train_data++;
		}

	}

    emit turn_complete(game_complete);
}


