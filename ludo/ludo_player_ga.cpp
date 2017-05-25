#include "ludo_player_ga.h"
#include <random>
#include <ctime>

ludo_player_ga::ludo_player_ga():
    pos_start_of_turn(16),
    pos_end_of_turn(16),
    dice_roll(0)
{
	srand(time(NULL));

	ranking = std::vector<int>(0);
	for(int i = 1; i<= POPULATION_SIZE; i++)
		for(int j = 0; j < POPULATION_SIZE - i + 1; j++)
			ranking.push_back(i);

	std::cout << "ranking size: " << ranking.size() << std::endl;

	//for(int i = 0; i< ranking.size(); i++)
	//	std::cout << ranking.at(i) << std::endl;




}

fann_type ludo_player_ga::calc_fitness()
{
	fann_type fit = wins;// + (finished - home)/10.0;
	//fann_type fit = left;
	finished = 0;
	home = 0;
	wins = 0;
	left = 0;

	return fit;

}

void ludo_player_ga::update_scores()
{

	for(int i = 0; i< 4; i++)
	{
		if(pos_end_of_turn[i] == 99)
			finished++;

		if(pos_end_of_turn[i] == -1)
			home++;

		left += pos_end_of_turn[i];

	}

}


void ludo_player_ga::create_new_neural_network()
{
	const unsigned int num_input = INPUT_SIZE;
     	const unsigned int num_output = 1;
  	const unsigned int num_layers = 3;
        const unsigned int num_neurons_hidden = 6;
	nn = fann_create_standard(num_layers, num_input, num_neurons_hidden, num_output);
        
   	fann_set_activation_function_hidden(nn, FANN_SIGMOID);	// try with FANN_SIGMOID_SYMMETRIC (tanh)
	fann_set_activation_function_output(nn, FANN_SIGMOID);	// try with something linear
	

}


void ludo_player_ga::init_pool()
{
	fann_type r_weights[17] = {-4.0, -3.5, -3.0, -2.5, -2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0};
	population = std::vector<individual>(POPULATION_SIZE);

	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_int_distribution<> distr(0,16);
	
	for(int i = 0; i< POPULATION_SIZE; i++)
	{
		//fann_randomize_weights(nn, -5.125, 5.125);
		//fann_get_weights(nn, population.at(i).weights);
		for(int j = 0; j< CHROMOSONE_SIZE; j++)
			population.at(i).weights[j] =r_weights[distr(eng)];

	}
	

	/*for(int i = 0; i< POPULATION_SIZE; i++)
	{
		for(int j = 0; j< CHROMOSONE_SIZE; j++)
		{
			population.at(i).weights[j] = distr(eng)/10000.0;
		}

	}*/




	for(int i = 0; i< CHROMOSONE_SIZE; i++)
		std::cout << "Weight: " << i << " = " << population.at(0).weights[i] << std::endl;

}

void ludo_player_ga::set_individual(int i)
{
	fann_set_weights(nn, population.at(i).weights);
	/*for(int j = 0; j< CHROMOSONE_SIZE; j++)
	{
		active_weights[j] = population.at(i).weights[j];
	}

	std::cout << "Active weights: " << std::endl;
	for(int j = 0; j< 1; j++)
		std::cout << active_weights[j] << '\t';

	std::cout << std::endl;
	*/
}

bool ludo_player_ga::sorting_func(individual i, individual j)
{
	return i.fitness>j.fitness;

}

void ludo_player_ga::sort_individuals()
{
	std::sort(population.begin(), population.end(), sorting_func);
	
}

std::vector<individual> ludo_player_ga::rank_selection()
{
	// total = 325
	//sort_individuals();
	
	// after sorting selct 10 using rank
	std::vector<individual> selected(0);
	std::vector<int> selection(0);

	/*std::vector<int> wheel(0);
	int total = 0;
	for(int i = 0; i< 25; i++)
	{
		int prob = population.at(i).fitness*100;
		total += prob;
		for(int j = 0; j< prob; j++)
			wheel.push_back(i);


	}*/

	

	int i = 0;
	while(i< PARENTS)
	{
		int sel = ranking.at(rand()%ranking.size()) -1;
		//int sel = wheel.at(rand()%wheel.size());		
		bool already = false;
		for(int j = 0; j< selection.size(); j++)
			if(sel == selection.at(j))
				already = true;
		if(!already)
		{	
			selection.push_back(sel);
			selected.push_back(population.at(sel));
			i++;
		}

	}
	
	


	//for(int j = 0; j < selection.size(); j++)
	//	std::cout << "Selected " << selection.at(j) << std::endl;
	

	return selected;

}

std::vector<individual> ludo_player_ga::cross_over(std::vector<individual> parents)
{
	fann_type r_weights[17] = {-4.0, -3.5, -3.0, -2.5, -2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0};
	std::vector<individual> childs(0);
	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_int_distribution<> distr(0,16);
	for(int i = 0; i< (CHILDREN/2); i++)
	{
		individual child1;
		individual child2;
		int parent1;
		int parent2;
		while(true)
		{
			parent1 = rand()%PARENTS;
			parent2 = rand()%PARENTS;
			if(parent1 != parent2)
				break;
		}
		
		/*std::cout << "Parent1: " << parent1 << std::endl;
		for(int kk = 0; kk < 13; kk++)
			std::cout << parents.at(parent1).weights[kk] << '\t';
		std::cout << std::endl;
		std::cout << "Parent2: " << parent2  << std::endl;
		for(int kk = 0; kk < 13; kk++)
			std::cout << parents.at(parent2).weights[kk] << '\t';
		std::cout << std::endl;*/
		double crossover_activation = (double)(rand()%1000)/1000.0;
		if(crossover_activation < crossover_rate)
		{
			int crosspoint = rand()%CHROMOSONE_SIZE;
			//std::cout << "Crosspoint is: " << crosspoint << std::endl;
			for(int j = 0; j< CHROMOSONE_SIZE; j++)
			{
				
				if(j < crosspoint)
				{
					child1.weights[j] = parents.at(parent1).weights[j];
					child2.weights[j] = parents.at(parent2).weights[j];
				}
				else
				{
					child1.weights[j] = parents.at(parent2).weights[j];
					child2.weights[j] = parents.at(parent1).weights[j];
				}

			}

			/*std::cout << "Child 1:" << std::endl;
			for(int kk = 0; kk< 1; kk++)
				std::cout << child1.weights[kk] << '\t';
			std::cout << std::endl;

			std::cout << "Child 2:" << std::endl;
			for(int kk = 0; kk< 1; kk++)
				std::cout << child2.weights[kk] << '\t';

			std::cout << std::endl;*/
			

		}
		else
		{	
			// Simple?
			child1 = parents.at(parent1);
			child2 = parents.at(parent2);

			// new random child
			//fann_randomize_weights(nn, -1.0, 1.0);
			//fann_get_weights(nn, child.weights);
			
		}

		// Mutation
		
		for(int j = 0; j< CHROMOSONE_SIZE; j++)
		{
			double mutation_activation = (double)(rand()%1000)/1000.0;
			if(mutation_activation < mutation_rate)
			{
				child1.weights[j] = r_weights[distr(eng)];
			}
			mutation_activation = (double)(rand()%1000)/1000.0;
			if(mutation_activation < mutation_rate)
			{
				child2.weights[j] = r_weights[distr(eng)];
			}
		}
		

		childs.push_back(child1);
		childs.push_back(child2);

	}

	return childs;


}

void ludo_player_ga::new_generation()
{
	// Select 10 parents from rank_selection

	std::vector<individual> parents = rank_selection();

	// Crossover 10 new child
	std::vector<individual> childs = cross_over(parents);

	// Elitism: generationsal
	std::vector<individual> survivors(0);
	for(int i = 0; i< SURVIVORS; i++)
		survivors.push_back(population.at(i));

	population.clear();

	for(int i = 0; i< SURVIVORS; i++)
		population.push_back(survivors.at(i));


	for(int i = 0; i< CHILDREN; i++)
	{
		population.push_back(childs.at(i));
	}


	/*std::cout << "New population weights: " << std::endl;
	for(int j = 0; j<1; j++)
		std::cout << population.at(0).weights[j] << '\t';

	std::cout << std::endl;
	for(int j = 0; j<1; j++)
		std::cout << population.at(1).weights[j] << '\t';*/

	std::cout << std::endl;
	


}


ludo_player_ga::~ludo_player_ga()
{
	print_player_pos();
	fann_destroy(nn);
	

}

std::vector<int> ludo_player_ga::possible_actions()
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


fann_type ludo_player_ga::is_home(int a_pos)
{
	if(a_pos < 0)
		return 1.0;
	else
		return 0.0;

}

fann_type ludo_player_ga::is_finished(int a_pos)
{
	if(a_pos == 99)
		return 1.0;
	else
		return 0.0;

}

fann_type ludo_player_ga::finish_area(int a_pos)
{
	if(a_pos >= 51 && a_pos <= 55)
		return 1.0;
	else
		return 0.0;

}

fann_type ludo_player_ga::distance_to_finish_now(int a_pos)
{
	if(a_pos == 99)
		return 0.0;
	else if(a_pos == -1)
		return 1.0;
	else
		return ((fann_type)56 - (fann_type)a_pos)/((fann_type)56.0);


}



fann_type ludo_player_ga::defender_now(int a_pos, int pos, std::vector<int> current_pos)
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
			return 1.0;
		else if(defenders == 2)
			return 1.0;
		else
			return 0.0;

	}


}

int ludo_player_ga::pos_next(int pos, int dice)
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

fann_type ludo_player_ga::hit_star(int a_pos)
{
	if(a_pos == 5 || a_pos == 18 || a_pos == 31 || a_pos == 44 || a_pos == 11 || a_pos == 24 || a_pos == 37 | a_pos == 50)
		return 1.0;
	else
		return 0.0;




}

int ludo_player_ga::globe_pos(int a_pos)
{
	// Check for enemy globes
	if(a_pos == 0 || a_pos == 8 || a_pos == 21 || a_pos == 34 || a_pos == 47)
		return 1;
	else if(a_pos == 13 || a_pos == 26 || a_pos == 39)
		return 2;
	else
		return 0;

}

int ludo_player_ga::friends_on_pos(int a_pos)
{
	int num = 0;
	for(int i = 0; i < 4; i++)
	{
		if(pos_start_of_turn[i] == a_pos)
			num++;

	}
	return num;

}


int ludo_player_ga::enemy_on_pos(int a_pos)
{
	int num = 0;
	for(int i = 4; i< 16; i++)
	{
		if(pos_start_of_turn[i] == a_pos)
			num++;

	}
	return num;

}


bool ludo_player_ga::die_next(int a_pos)
{
	if(a_pos <= 0)
		return false;
	else if(globe_pos(a_pos) && enemy_on_pos(a_pos))
		return true;
	else if(enemy_on_pos(a_pos) >= 2 && a_pos <=50)
		return true;
	else
		return false; 

}




bool ludo_player_ga::kill_next(int a_pos)
{

	// Kill this turn
	if(globe_pos(a_pos) == 0 && enemy_on_pos(a_pos) == 1)
		return true;
	else if(a_pos == 0 && enemy_on_pos(a_pos) > 0)
		return true;
	else 		
		return false;

}

fann_type ludo_player_ga::safe_now(int a_pos)
{
	if(globe_pos(a_pos) >= 1)
		return 1.0;
	else
	{
		
		return 0.0;	// Check for enemies nearby?

	}


}


fann_type ludo_player_ga::distance_to_enemy_behind(int a_pos)
{
	int nearest_distance = 50;
	for(int i = 4; i< 16; i++)
		if(a_pos > pos_start_of_turn[i] && (pos_start_of_turn[i] >= 0) && (a_pos >= 0) && (a_pos <= 50))
		{
			int distance = a_pos - pos_start_of_turn[i];
			if(distance < nearest_distance)
				nearest_distance = distance;

		}	
	/*if(nearest_distance <= 6)
		return 1.0;
	else
		return 0.0;
	*/
	return ((fann_type)nearest_distance)/50.0;
	

}



fann_type ludo_player_ga::distance_to_enemy_front(int a_pos)
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

fann_type ludo_player_ga::num_home()
{
	int num = 0;
	for(int i = 0; i< 4; i++)
	{
		if(pos_start_of_turn[i] == -1)
			num++;

	}

	return num;

}

fann_type ludo_player_ga::num_finished()
{
	int num = 0;
	for(int i = 0; i<4; i++)
	{
		if(pos_start_of_turn[i] == 99)
			num++;

	}

	return num;


}

fann_type ludo_player_ga::num_enemies()
{
	int num = 0;
	for(int i = 4; i<16; i++)
	{
		if(pos_start_of_turn[i] >= 0 && pos_start_of_turn[i] <=50)
			num++;


	}

	return num;


}

void ludo_player_ga::move_piece(int pos, bool &kill)
{
	int new_pos = pos_next(pos, dice_roll);
	kill = false;
	if(die_next(new_pos))
	{
		pos_start_of_turn[pos] = -1;
	}
	else if(kill_next(new_pos))
	{
		kill = true;
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

fann_type ludo_player_ga::last_part(int a_pos)
{
	if(a_pos >=51 && a_pos <99)
		return 1.0;
	else
		return 0.0;

}


std::vector<fann_type> ludo_player_ga::feature_values(int pos)
{
	std::vector<fann_type> res(0);
	// General state now

	// Piece specific state now
	int a_pos = pos_start_of_turn[pos];
	res.push_back(is_home(a_pos));					// Move out
	res.push_back(last_part(a_pos));				// Is in the last part


	// Piece specific state next
	std::vector<int> pos_start_of_turn_orig = pos_start_of_turn;
	bool killer;
	move_piece(pos, killer);
	a_pos = pos_start_of_turn[pos];
	res.push_back(is_home(a_pos) && !res.at(0));					// Die
	res.push_back(is_finished(a_pos));				// Move into goal
	res.push_back(safe_now(a_pos));					// Move onto globus
	res.push_back(hit_star(a_pos));					// Land on star
	res.push_back(last_part(a_pos) && !res.at(1));				// Will move into the last part
	res.push_back((fann_type)killer);				// Kill opponent


	pos_start_of_turn = pos_start_of_turn_orig;

	//std::cout << "Feature values: " << res.at(0) << " , " << res.at(1) << " , "<< res.at(2) << " , "<< res.at(3) << " , "<< res.at(4) << " , "<< res.at(5) << " , "<< res.at(6) << " , "<< res.at(7) << std::endl;
	return res;


}

void ludo_player_ga::print_player_pos()
{
	std::cout << "Player Positions: \n";
	for(int i = 0; i< 16; i++)
		std::cout << pos_start_of_turn[i] << " , ";

	std::cout << std::endl;


}



void ludo_player_ga::save_neural_network(std::string path)
{
	fann_save(nn, path.c_str());

}

void ludo_player_ga::load_neural_network(std::string path)
{
	nn = fann_create_from_file(path.c_str());
	fann_set_training_algorithm(nn, FANN_TRAIN_BATCH);
   	fann_set_activation_function_hidden(nn, FANN_SIGMOID_SYMMETRIC);	// try with FANN_SIGMOID_SYMMETRIC (tanh)
	fann_set_activation_function_output(nn, FANN_SIGMOID_SYMMETRIC);	// try with something linear


}




int ludo_player_ga::find_best_action(std::vector<int> actions, fann_type max_a0, fann_type max_a1, fann_type max_a2, fann_type max_a3)
{
	fann_type max = -1000000000;
	int action = -1;
	for(int i = 0; i< actions.size(); i++)
	{
		int check = actions.at(i);
		switch(check)
		{
			case 0:
				if(max_a0 > max)
				{
					max = max_a0;
					action = check;

				}
				break;

			case 1:

				if(max_a1 > max)
				{
					max = max_a1;
					action = check;

				}
				break;

			
			case 2:

				if(max_a2 > max)
				{
					max = max_a2;
					action = check;

				}
				break;


			case 3:

				if(max_a3 > max)
				{
					max = max_a3;
					action = check;

				}
				break;

			default:
				action = -1;
				break;


		}
	}

	

	return action;


}

fann_type ludo_player_ga::nn_value(std::vector<fann_type> inputs)
{
	fann_type res = 0;

	//	for(int i = 0; i< CHROMOSONE_SIZE; i++)
	//		res += inputs.at(i) * active_weights[i];


	fann_type input[INPUT_SIZE];
	for(int i = 0; i< INPUT_SIZE; i++)
		input[i] = inputs.at(i);

	fann_type *output;
	
	output = fann_run(nn, input);
	res = output[0];

	if(std::isnan(res))
		std::cout << "Q-value is nan!" << std::endl;
	//delete[] q_value;
	return res;

}

	


int ludo_player_ga::make_decision_ga()
{

	//print_player_pos();

	////////////
	/////// Current State
	////////////

	std::vector<fann_type> current_state_a0 = feature_values(0);
	std::vector<fann_type> current_state_a1 = feature_values(1);
	std::vector<fann_type> current_state_a2 = feature_values(2);
	std::vector<fann_type> current_state_a3 = feature_values(3);

	/*std::cout << "Feature_valus a1: " << std::endl;
	for(int i = 0; i<current_state_a0.size();i++)
		std::cout << current_state_a0.at(i) << '\t' << std::endl;
	std::cout << std::endl;
	*/

	fann_type current_state_a0_value = nn_value(current_state_a0);
	fann_type current_state_a1_value = nn_value(current_state_a1);
	fann_type current_state_a2_value = nn_value(current_state_a2);
	fann_type current_state_a3_value = nn_value(current_state_a3);

	if(DEBUG)
	{
		std::cout << "value for a0: " << current_state_a0_value << std::endl;
		std::cout << "value for a1: " << current_state_a1_value << std::endl;
		std::cout << "value for a2: " << current_state_a2_value << std::endl;
		std::cout << "value for a3: " << current_state_a3_value << std::endl;
	}


	// Find the best action
	std::vector<int> pos_actions = possible_actions();
	int best_action = find_best_action(pos_actions, current_state_a0_value, current_state_a1_value, current_state_a2_value, current_state_a3_value);
	if(DEBUG)
	{
		std::cout << "Possible actions: ";
		for(int i = 0; i< pos_actions.size(); i++)
		{
			std::cout << pos_actions.at(i)  << " , "; 
		}
		std::cout << std::endl;
	}

	//std::cout << "Best _action: " << best_action << std::endl;
	// if no possible actions -> return
	if(best_action == -1)
	{
		for(int i = 0; i< 4; i++)
		{
			if(pos_start_of_turn[i] == -1)
				best_action = i;
		}
		return best_action;

	}

	

	//std::cout << "Selected action: " << best_action << std::endl;
	//std::cout << std::endl;
	//std::string test;
	//std::cin >> test;
	return best_action;

}


void ludo_player_ga::start_turn(positions_and_dice relative){
    pos_start_of_turn = relative.pos;
    dice_roll = relative.dice;
    decision = make_decision_ga();
    emit select_piece(decision);
}

void ludo_player_ga::post_game_analysis(std::vector<int> relative_pos){
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


