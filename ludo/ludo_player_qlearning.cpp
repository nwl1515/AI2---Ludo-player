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


	training_data = new struct fann_train_data;
	//training_data->num_data = 2;
	training_data->num_input = 9;
	training_data->num_output = 1;
	training_data->input = input_data;
	training_data->output = output_data;

	srand(time(NULL));


	
	for(int i = 0; i< 100000; i++)
	{
		input_data[i] = new fann_type[9];
		output_data[i] = new fann_type[1];
	}



}

void ludo_player_qlearning::reset_states()
{
	


}

void ludo_player_qlearning::create_new_neural_network()
{
	const unsigned int num_input = 9;
     	const unsigned int num_output = 1;
  	const unsigned int num_layers = 3;
        const unsigned int num_neurons_hidden = 50;
	q_approximator = fann_create_standard(num_layers, num_input, num_neurons_hidden, num_output);
        
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

fann_type ludo_player_qlearning::is_finished(int a_pos)
{
	if(a_pos == 99)
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


std::vector<fann_type> ludo_player_qlearning::feature_values(int pos)
{
	std::vector<fann_type> res(0);
	res.push_back(num_home()/4.0);
	res.push_back(num_finished()/4.0);

	int a_pos = pos_start_of_turn[pos];
	res.push_back(is_home(a_pos));
	res.push_back(is_finished(a_pos));
	res.push_back(safe_now(a_pos));
	res.push_back(defender_now(a_pos, pos, pos_start_of_turn));
	res.push_back(distance_to_finish_now(a_pos));
	res.push_back(distance_to_enemy_behind(a_pos));
	res.push_back(distance_to_enemy_front(a_pos));


	return res;


}

void ludo_player_qlearning::move_piece(int pos)
{
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

		
	training_data->num_data = num_train_data;
	//fann_train_on_data(q_approximator, training_data, 3000, 0, 0.00001);
	fann_train_epoch(q_approximator, training_data);	
	num_train_data = 0;
	learning_rate = learning_rate*0.95;
	fann_set_learning_rate(q_approximator, learning_rate);
	

	
}

fann_type ludo_player_qlearning::calc_reward(std::vector<fann_type> old_state, std::vector<fann_type> state, int old_pos, int new_pos, bool kill)
{
	int reward = 0;
	// win is not included here
	if(kill)
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

		if(old_pos >= 0 && new_pos == -1)
		{
			//got killed
			reward -= 2;

		}

		if(old_pos == -1 && new_pos >= 0)
		{
			// Move out
			reward += 2;
		}

		if(old_pos != 99 && new_pos == 99)
		{
			// Finished one piece
			reward += 5;
		
		}

	}

	return reward;
}

int ludo_player_qlearning::find_best_action(std::vector<int> actions, fann_type max_a0, fann_type max_a1, fann_type max_a2, fann_type max_a3)
{
	int action = -1;

	

	return action;


}

fann_type ludo_player_qlearning::q_approx(std::vector<fann_type> inputs)
{
	fann_type input[9];
	//std::cout << "Inputs size: " << inputs.size() << std::endl;
	
	for(int j = 0; j< inputs.size() ; j++)
	{
		input[j] = inputs.at(j);

	}

	fann_type *q_value = fann_run(q_approximator, input);
	fann_type res = q_value[0];
	//std::cout << "q_value: " << res << std::endl;
	if(std::isnan(res))
		std::cout << "Q-value is nan!" << std::endl;
	//delete[] q_value;
	return res;

}

fann_type ludo_player_qlearning::max_q(fann_type a0, fann_type a1, fann_type a2, fann_type a3)
{
	fann_type max = -100000;

	if(a0 > max)
		max = a0;

	if(a1 > max)
		max = a1;

	if(a2 > max)
		max = a2;

	if(a3 > max)
		max = a3;

	return max;


}	

bool ludo_player_qlearning::win_game()
{
	bool win = true;
	for(int i = 0; i< 4; i++)
	{
		if(pos_start_of_turn[i] != 99)
			win = false;

	}

	return win;

}

int ludo_player_qlearning::make_decision_qlearning()
{

	//print_player_pos();

	////////////
	/////// Current State
	////////////
	current_state_a0 = feature_values(0);
	current_state_a1 = feature_values(1);
	current_state_a2 = feature_values(2);
	current_state_a3 = feature_values(3);

	fann_type current_state_a0_Q = q_approx(current_state_a0);
	fann_type current_state_a1_Q = q_approx(current_state_a1);
	fann_type current_state_a2_Q = q_approx(current_state_a2);
	fann_type current_state_a3_Q = q_approx(current_state_a3);


					// Only do this while training??????
	////////////
	///////	Next State
	////////////
	pos_start_of_turn_orig = pos_start_of_turn;

	// Try to move piece 0
	move_piece(0);
	winning_a0 = win_game();
	next_state_a0_a0 = feature_values(0);
	next_state_a0_a1 = feature_values(1);
	next_state_a0_a2 = feature_values(2);
	next_state_a0_a3 = feature_values(3);
	pos_start_of_turn = pos_start_of_turn_orig;

	// Try to move piece 1
	move_piece(1);
	winning_a1 = win_game();
	next_state_a1_a0 = feature_values(0);
	next_state_a1_a1 = feature_values(1);
	next_state_a1_a2 = feature_values(2);
	next_state_a1_a3 = feature_values(3);
	pos_start_of_turn = pos_start_of_turn_orig;

	// Try to move piece 1
	move_piece(2);
	winning_a2 = win_game();
	next_state_a2_a0 = feature_values(0);
	next_state_a2_a1 = feature_values(1);
	next_state_a2_a2 = feature_values(2);
	next_state_a2_a3 = feature_values(3);
	pos_start_of_turn = pos_start_of_turn_orig;

	// Try to move piece 1
	move_piece(3);
	winning_a3 = win_game();
	next_state_a3_a0 = feature_values(0);
	next_state_a3_a1 = feature_values(1);
	next_state_a3_a2 = feature_values(2);
	next_state_a3_a3 = feature_values(3);
	pos_start_of_turn = pos_start_of_turn_orig;


	// Find q values for piece 0
	fann_type next_state_a0_a0_Q = q_approx(next_state_a0_a0);
	fann_type next_state_a0_a1_Q = q_approx(next_state_a0_a1);
	fann_type next_state_a0_a2_Q = q_approx(next_state_a0_a2);
	fann_type next_state_a0_a3_Q = q_approx(next_state_a0_a3);

	// Find q values for piece 1
	fann_type next_state_a1_a0_Q = q_approx(next_state_a1_a0);
	fann_type next_state_a1_a1_Q = q_approx(next_state_a1_a1);
	fann_type next_state_a1_a2_Q = q_approx(next_state_a1_a2);
	fann_type next_state_a1_a3_Q = q_approx(next_state_a1_a3);

	// Find q values for piece 2
	fann_type next_state_a2_a0_Q = q_approx(next_state_a2_a0);
	fann_type next_state_a2_a1_Q = q_approx(next_state_a2_a1);
	fann_type next_state_a2_a2_Q = q_approx(next_state_a2_a2);
	fann_type next_state_a2_a3_Q = q_approx(next_state_a2_a3);

	// Find q values for piece 3
	fann_type next_state_a3_a0_Q = q_approx(next_state_a3_a0);
	fann_type next_state_a3_a1_Q = q_approx(next_state_a3_a1);
	fann_type next_state_a3_a2_Q = q_approx(next_state_a3_a2);
	fann_type next_state_a3_a3_Q = q_approx(next_state_a3_a3);


	// Find max for each piece
	fann_type max_a0 = max_q(next_state_a0_a0_Q, next_state_a0_a1_Q, next_state_a0_a2_Q, next_state_a0_a3_Q);
	fann_type max_a1 = max_q(next_state_a1_a0_Q, next_state_a1_a1_Q, next_state_a1_a2_Q, next_state_a1_a3_Q);
	fann_type max_a2 = max_q(next_state_a2_a0_Q, next_state_a2_a1_Q, next_state_a2_a2_Q, next_state_a2_a3_Q);
	fann_type max_a3 = max_q(next_state_a3_a0_Q, next_state_a3_a1_Q, next_state_a3_a2_Q, next_state_a3_a3_Q);

	// Find the best action
	std::vector<int> pos_actions = possible_actions();
	int best_action = find_best_action(pos_actions, max_a0, max_a1, max_a2, max_a3);

	if(best_action == -1)
	{
		for(int i = 0; i< 4; i++)
		{
			if(pos_start_of_turn[i] == -1)
				best_action = i;
		}
		return best_action;

	}
	else
	{

		if(training)
		{
			if(best_action == 0)
			{
				if(winning_a0)
					output_data[num_train_data][0] = reward + discount_factor*max_a0;
				else
					output_data[num_train_data][0] = 50;
				output_data[num_train_data+1][0] = current_state_a1_Q;
				output_data[num_train_data+2][0] = current_state_a2_Q;
				output_data[num_train_data+3][0] = current_state_a3_Q;
			}
			else if(best_action == 1)
			{
				output_data[num_train_data][0] = current_state_a0_Q;
				if(winning_a1)
					output_data[num_train_data+1][0] = reward + discount_factor*max_a1;
				else
					output_data[num_train_data+1][0] = 50;
				output_data[num_train_data+2][0] = current_state_a2_Q;
				output_data[num_train_data+3][0] = current_state_a3_Q;
			}
			else if(best_action == 2)
			{
				output_data[num_train_data][0] = current_state_a0_Q;
				output_data[num_train_data+1][0] = current_state_a1_Q;
				if(winning_a2)
					output_data[num_train_data+2][0] = reward + discount_factor*max_a2;
				else
					output_data[num_train_data+2][0] = 50;
				output_data[num_train_data+3][0] = current_state_a3_Q;
			}
			else if(best_action == 3)
			{
				output_data[num_train_data][0] = current_state_a0_Q;
				output_data[num_train_data+1][0] = current_state_a1_Q;
				output_data[num_train_data+2][0] = current_state_a2_Q;
				if(winning_a3)
					output_data[num_train_data+3][0] = reward + discount_factor*max_a3;
				else
					output_data[num_train_data+3][0] = 50;
			}

			for(int i = 0; i< current_state_a0.size() ; i++)
			{
				input_data[num_train_data][i] = current_state_a0.at(i);
				input_data[num_train_data+1][i] = current_state_a1.at(i);
				input_data[num_train_data+2][i] = current_state_a2.at(i);
				input_data[num_train_data+3][i] = current_state_a3.at(i);

			}
			num_train_data = num_train_data+4;


			double exploration_activation = (double)(rand()%1000)/1000.0;
			if(exploration_activation < exploring_rate)
			{
				int randnum = rand()%(pos_actions.size());
				best_action = pos_actions.at(randnum);

			}

		}
		
		return best_action;




	}
	


	
	





	int action = 0;
	if(pos_actions.size() >0)
	{

	
		
		if(training)
		{
		



			

		}

	}

	//std::cout << "Selected action: " << action << std::endl;
	//std::cout << std::endl;
	return action;

}


void ludo_player_qlearning::start_turn(positions_and_dice relative){
    pos_start_of_turn = relative.pos;
    dice_roll = relative.dice;
    decision = make_decision_qlearning();
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

	}

    emit turn_complete(game_complete);
}


