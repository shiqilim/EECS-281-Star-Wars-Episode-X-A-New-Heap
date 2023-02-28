/// PROJECT IDENTIFIER: AD48FB4835AF347EB0CA8009E24C3B13F8519882
#ifndef BATTALION_H
#define BATTALION_H

#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <deque>
#include <stack>
#include <queue>
#include <utility>
#include <vector>
#include "P2random.h"
using namespace std;

struct deployment {
	uint32_t f; //force sensitivity
	mutable int num_troop; //number of troops
	uint32_t time_stamp; //time stamp
	uint32_t general_id; //general ID
	uint32_t unique_id; //unique ID
};

struct general {
	uint32_t jedi;
	uint32_t sith;
	uint32_t left;
	uint32_t total;
};

//functor for Jedi
struct j_functor {
	bool operator()(const deployment& left, const deployment& right)const {
		if (right.f < left.f) {
			return true;
		}
		else if (right.f == left.f) {
			if (right.time_stamp < left.time_stamp) {
				return true;
			}
			else if (right.time_stamp == left.time_stamp) {
				return right.unique_id < left.unique_id ? true : false;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
};

//functor for Sith
struct s_functor {
	bool operator()(const deployment& left, const deployment& right) {
		if (right.f > left.f) {
			return true;
		}
		else if (right.f == left.f) {
			if (right.time_stamp < left.time_stamp) {
				return true;
			}
			else if (right.time_stamp == left.time_stamp) {
				return right.unique_id < left.unique_id ? true : false;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
};

//functor for median
struct m_functor {
	bool operator()(int left, int right) {
		return right < left ? true : false;
	}
};

struct planet {
	priority_queue<deployment, vector<deployment>, j_functor> Jedi;
	priority_queue<deployment, vector<deployment>, s_functor> Sith;
};

//type for median, contains two small vector
struct mtype {
	priority_queue <int> lower;
	priority_queue <int, vector<int>, m_functor > higher;
};

enum class State {
	Initial, SeenOne, SeenBoth, Better
};

struct movie_type {
	int JediTime;
	int JediForce;
	int SithTime;
	int SithForce;
	int BetterTime;
	int BetterForce;
};

struct Movie {
	State attack = State::Initial;
	State ambush = State::Initial;
	movie_type am;//ambush
	movie_type at;//attack
};


class Battalion {

public:
	/**************VARIABLES*****************/
	uint32_t num_general; //number of generals
	uint32_t num_planet; //number of planet
	uint32_t num_battle; //count of battle
	uint32_t unique; //unique id for deployment
	uint32_t seed; //for PR
	uint32_t num_deploy; //for PR
	uint32_t arr_rate; //for PR
	bool format; //false is DL, true is PR
	bool verbose; //default false
	bool median; //default false
	bool g_eval; //default false
	bool watcher; //default false

	/**************CONSTRUCTOR*****************/
	Battalion() :num_general(0), num_planet(0), num_battle(0), unique(0),
		seed(0), num_deploy(0), arr_rate(0), format(false), verbose(false),
		median(false), g_eval(false), watcher(false) {}

	void getMode(int argc, char* argv[]) {
		opterr = false; // Let us handle all error output for command line options
		int choice;
		int option_index = 0;
		option long_options[] = {
			{"verbose",no_argument,nullptr,'v'},
			{"median",no_argument,nullptr,'m'},
			{"general-eval",no_argument,nullptr,'g'},
			{"watcher",no_argument,nullptr,'w'},
			{ nullptr, 0, nullptr, '\0' }
		};

		while ((choice = getopt_long(argc, argv, "vmgw", long_options, &option_index)) != -1) {
			switch (choice) {
			case 'v':
				verbose = true;
				break;
			case 'm':
				median = true;
				break;
			case 'g':
				g_eval = true;
				break;
			case 'w':
				watcher = true;
				break;
			default:
				cerr << "Error: invalid option" << endl;
				exit(1);
			} // switch
		} // while
	} // getMode()

	//read in mode, num_general, num_planet
	void init() {
		string junk;
		getline(cin, junk);
		cin >> junk >> junk;
		if (junk == "PR") {
			format = true;
			cin >> junk >> num_general;
			cin >> junk >> num_planet;
			cin >> junk >> seed;
			cin >> junk >> num_deploy;
			cin >> junk >> arr_rate;
		}
		else {
			cin >> junk >> num_general;
			cin >> junk >> num_planet;
		}
	}

	void read_deploy(istream& is, vector<planet>& universe, vector<general>& general_con, vector<Movie>& state_con) {
		vector<mtype> median_con;
		median_con.resize(num_planet);
		string jedi_sith;
		uint32_t prev_time_stamp = 0;
		uint32_t time_stamp;
		uint32_t general_id;
		uint32_t planet_id;
		uint32_t force;
		int troops;
		char junk;

		while (is >> time_stamp >> jedi_sith >> junk >>
			general_id >> junk >> planet_id >> junk >> force >> junk >> troops) {
			check_error_ID(planet_id, general_id);
			check_error_force_troop(force, troops);
			if (time_stamp < prev_time_stamp) {
				cerr << "Invalid deployment\n";
				exit(1);
			}

			if (watcher) {
				ambush_movie(state_con[planet_id], jedi_sith.at(0), force, time_stamp);
				attack_movie(state_con[planet_id], jedi_sith.at(0), force, time_stamp);
			}

			if (time_stamp > prev_time_stamp) {
				if (median) {
					for (uint32_t i = 0; i < num_planet; i++) {
						if (median_con[i].lower.size()!=0 ) {
							cout << "Median troops lost on planet " << i << " at time "
								<< prev_time_stamp << " is " << median_func(median_con[i]) << ".\n";
						}
					}
				}
				prev_time_stamp = time_stamp;
			}
			
			deployment deploy{ force,troops,time_stamp,general_id,unique };
			general_con.at(general_id).total += troops;
			general_con.at(general_id).left += troops;

			if (jedi_sith.at(0) == 'J') {
				general_con.at(general_id).jedi += troops;
				universe.at(planet_id).Jedi.push(deploy);
			}
			else {
				general_con.at(general_id).sith += troops;
				universe.at(planet_id).Sith.push(deploy);
			}
			battle(universe, general_con, median_con, planet_id, prev_time_stamp, time_stamp);
			unique++;	
		}//while

		//display last time stamp
		if (median) {
			for (uint32_t i = 0; i < num_planet; i++) {
				if (median_con[i].lower.size() != 0) {
					cout << "Median troops lost on planet " << i << " at time "
						<< time_stamp << " is " << median_func(median_con[i]) << ".\n";
				}
			}
		}

	}

	uint32_t median_func(mtype& m) {
		if (m.higher.size() == m.lower.size()) {
			return (m.higher.top() + m.lower.top()) / 2;
		}
		else if (m.higher.size() > m.lower.size()) {
			return m.higher.top();
		}
		else {
			return m.lower.top();
		}
	}

	void battle(vector<planet>& universe, vector<general>& general_con, vector<mtype>& median_con, uint32_t planet_id, uint32_t t1, uint32_t t2) {
		while (universe.at(planet_id).Sith.size() != 0 && universe.at(planet_id).Jedi.size() != 0
			&& universe.at(planet_id).Jedi.top().f <= universe.at(planet_id).Sith.top().f ) {
			if (compare_troop(universe.at(planet_id).Jedi.top(), universe.at(planet_id).Sith.top())) {
				if (verbose) {
					print_verbose(universe, planet_id, true);
				}
				if (t2 >= t1 && median) {
					uint32_t add = universe.at(planet_id).Jedi.top().num_troop * 2;
					add_median(median_con[planet_id], add);
				}
				if (g_eval) {
					general_con_update(universe, general_con, planet_id, true);
				}

				if (universe.at(planet_id).Jedi.top().num_troop != universe.at(planet_id).Sith.top().num_troop) {
					universe.at(planet_id).Sith.top().num_troop -= universe.at(planet_id).Jedi.top().num_troop;
					universe.at(planet_id).Jedi.pop();
				}
				else {
					universe.at(planet_id).Jedi.pop();
					universe.at(planet_id).Sith.pop();
				}

				num_battle++;
			}
			else {
				if (verbose) {
					print_verbose(universe, planet_id, false);
				}
				if (t2 >= t1 && median) {
					uint32_t add = universe.at(planet_id).Sith.top().num_troop * 2;
					add_median(median_con[planet_id], add);
				}
				if (g_eval) {
					general_con_update(universe, general_con, planet_id, false);
				}

				universe.at(planet_id).Jedi.top().num_troop -= universe.at(planet_id).Sith.top().num_troop;
				universe.at(planet_id).Sith.pop();

				num_battle++;
			}
		}
	}

	void add_median(mtype& m, int num) {
		if (m.lower.empty() && m.higher.empty()) {
			m.lower.push(num);
		}
		else if (num <= m.lower.top()) {
			m.lower.push(num);
			rebalance_median(m);
		}
		else if (num > m.lower.top()) {
			m.higher.push(num);
			rebalance_median(m);
		}
	}

	void rebalance_median(mtype& m) {
		int h = static_cast<int>(m.higher.size());
		int l = static_cast<int>(m.lower.size());
		if ((l - h) >= 2) {
			m.higher.push(m.lower.top());
			m.lower.pop();
		}
		else if ((h-l) >= 2) {
			m.lower.push(m.higher.top());
			m.higher.pop();
		}
	}

	void ambush_movie(Movie& m, char c, int force, int time_stamp) {
		switch (m.ambush) {
		case State::Initial:
			if (c == 'S') {
				m.am.SithForce = force;
				m.am.SithTime = time_stamp;
				m.ambush = State::SeenOne;
			}
			break;
		case State::SeenOne:
			if (c == 'J') {
				if (force <= m.am.SithForce) {
					m.am.JediForce = force;
					m.am.JediTime = time_stamp;
					m.ambush = State::SeenBoth;
				}
			}
			else if (c == 'S') {
				if (force > m.am.SithForce) {
					m.am.SithForce = force;
					m.am.SithTime = time_stamp;
				}
			}
			break;
		case State::SeenBoth:
			if (c == 'J') {
				if (force < m.am.JediForce) {
					m.am.JediForce = force;
					m.am.JediTime = time_stamp;
				}
			}
			else if (c == 'S') {
				if (force > m.am.SithForce) {
					m.am.BetterForce = force;
					m.am.BetterTime = time_stamp;
					m.ambush = State::Better;
				}
			}
			break;
		case State::Better:
			if (c == 'S') {
				if (force > m.am.BetterForce) {
					m.am.BetterForce = force;
					m.am.BetterTime = time_stamp;
				}
			}
			else if (c == 'J') {
				if (force <= m.am.BetterForce) {
					if ((m.am.BetterForce - force) > (m.am.SithForce - m.am.JediForce)) {
						m.am.SithForce = m.am.BetterForce;
						m.am.SithTime = m.am.BetterTime;
						m.am.JediForce = force;
						m.am.JediTime = time_stamp;
						m.ambush = State::SeenBoth;
					}
				}
			}
			break;
		}
	}

	void attack_movie(Movie& m, char c, int force, int time_stamp) {
		switch (m.attack) {
		case State::Initial:
			if (c == 'J') {
				m.at.JediForce = force;
				m.at.JediTime = time_stamp;
				m.attack = State::SeenOne;
			}
			break;
		case State::SeenOne:
			if (c == 'J') {
				if (force < m.at.JediForce) {
					m.at.JediForce = force;
					m.at.JediTime = time_stamp;
				}
			}
			else if (c == 'S') {
				if (force >= m.at.JediForce) {
					m.at.SithForce = force;
					m.at.SithTime = time_stamp;
					m.attack = State::SeenBoth;
				}
			}
			break;
		case State::SeenBoth:
			if (c == 'S') {
				if (force > m.at.SithForce) {
					m.at.SithForce = force;
					m.at.SithTime = time_stamp;
				}
			}
			else if (c == 'J') {
				if (force < m.at.JediForce) {
					m.at.BetterForce = force;
					m.at.BetterTime = time_stamp;
					m.attack = State::Better;
				}
			}
			break;
		case State::Better:
			if (c == 'J') {
				if (force < m.at.BetterForce) {
					m.at.BetterForce = force;
					m.at.BetterTime = time_stamp;
				}
			}
			else if (c == 'S') {
				if (force >= m.at.BetterForce) {
					if ((force - m.at.BetterForce) > (m.at.SithForce - m.at.JediForce)) {
						m.at.SithForce = force;
						m.at.SithTime = time_stamp;
						m.at.JediForce = m.at.BetterForce;
						m.at.JediTime = m.at.BetterTime;
						m.attack = State::SeenBoth;
					}
				}
			}
			break;
		}
	}

	bool compare_troop(const deployment& left, const deployment& right) {
		return left.num_troop <= right.num_troop;
	}

	void general_con_update(vector<planet>& universe, vector<general>& general_con, uint32_t planet_id, bool t) {
		if (t) {
			uint32_t general_id_jedi = universe.at(planet_id).Jedi.top().general_id;
			uint32_t general_id_sith = universe.at(planet_id).Sith.top().general_id;
			general_con.at(general_id_jedi).left -= universe.at(planet_id).Jedi.top().num_troop;
			general_con.at(general_id_sith).left -= universe.at(planet_id).Jedi.top().num_troop;
		}
		else {
			uint32_t general_id_sith = universe.at(planet_id).Sith.top().general_id;
			uint32_t general_id_jedi = universe.at(planet_id).Jedi.top().general_id;
			general_con.at(general_id_sith).left -= universe.at(planet_id).Sith.top().num_troop;
			general_con.at(general_id_jedi).left -= universe.at(planet_id).Sith.top().num_troop;
		}
	}

	void print_verbose(vector<planet>& universe, uint32_t planet_id, bool t) {
		int n = t ? universe.at(planet_id).Jedi.top().num_troop * 2 : universe.at(planet_id).Sith.top().num_troop * 2;
		cout << "General " << universe.at(planet_id).Sith.top().general_id
			<< "'s battalion attacked General "
			<< universe.at(planet_id).Jedi.top().general_id << "'s battalion on planet "
			<< planet_id << ". " << n << " troops were lost.\n";
	}

	void check_error_ID(uint32_t planet_id, uint32_t general_id) {
		if ((int)planet_id < 0 || planet_id >= num_planet || (int)general_id < 0 || general_id >= num_general) {
			cerr << "Invalid deployment\n";
			exit(1);
		}
	}

	void check_error_force_troop(uint32_t force, uint32_t troops) {
		if (force <= 0 || troops <= 0) {
			cerr << "Invalid deployment\n";
			exit(1);
		}
	}

	void start() {
		init();
		vector<planet> universe;
		vector<general> general_con;
		vector<Movie> state_con;
		state_con.resize(num_planet);
		universe.resize(num_planet);
		general_con.resize(num_general);

		cout << "Deploying troops...\n";
		if (format) {
			stringstream ss;
			P2random::PR_init(ss, seed, num_general, num_planet, num_deploy, arr_rate);
			read_deploy(ss, universe, general_con, state_con);
		}
		else {
			read_deploy(cin, universe, general_con, state_con);
		}
		cout << "---End of Day---\n";
		cout << "Battles: " << num_battle << "\n";

		if (g_eval) {
			print_eval(general_con);
		}
		if (watcher) {
			print_movie(state_con);
		}
	}

	void print_eval(vector<general>& general_con) {
		cout << "---General Evaluation---\n";
		for (uint32_t i = 0; i < num_general; i++) {
			cout << "General " << i << " deployed " << general_con.at(i).jedi
				<< " Jedi troops and " << general_con.at(i).sith
				<< " Sith troops, and " << general_con.at(i).left
				<< "/" << general_con.at(i).total << " troops survived.\n";
		}
	}

	void print_movie(vector<Movie>& state_con) {
		cout << "---Movie Watcher---\n";
		for (int i = 0; i < static_cast<int>(num_planet); i++) {
			if (state_con[i].ambush == State::Better || state_con[i].ambush == State::SeenBoth) {
				cout << "A movie watcher would enjoy an ambush on planet " << i
					<< " with Sith at time " << state_con[i].am.SithTime << " and Jedi at time "
					<< state_con[i].am.JediTime << " with a force difference of "
					<< state_con[i].am.SithForce - state_con[i].am.JediForce << ".\n";
			}
			else {
				cout << "A movie watcher would enjoy an ambush on planet " << i
					<< " with Sith at time -1 and Jedi at time -1 with a force difference of 0.\n";
			}

			if (state_con[i].attack == State::Better || state_con[i].attack == State::SeenBoth) {
				cout << "A movie watcher would enjoy an attack on planet " << i
					<< " with Jedi at time " << state_con[i].at.JediTime << " and Sith at time "
					<< state_con[i].at.SithTime << " with a force difference of "
					<< state_con[i].at.SithForce - state_con[i].at.JediForce << ".\n";
			}
			else {
				cout << "A movie watcher would enjoy an attack on planet " << i
					<< " with Jedi at time -1 and Sith at time -1 with a force difference of 0.\n";
			}
		}
	}
};
#endif // BATTALION_H
