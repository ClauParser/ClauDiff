
// ClauDiff
#include <iostream>
#include <utility>
#include <vector>

#include <map>

#include "clau_parser.h"


class _Out {
private:
	std::string LOG_FILE_NAME = "clautext_log.txt";
	std::ofstream outFile;
			
	long policy = 0; // default 0 - only console, 1 - only file, 2 - file and console. else - none
public:
	_Out& operator = (const _Out& other) {
		policy = other.policy;
		return *this;
	}

	template<class T>
	_Out& operator << (const T& data)
	{
		if (0 == policy || 2 == policy) {
			std::cout << data;
		}
		if (1 == policy || 2 == policy) {
			//std::ofstream outFile;
			//outFile.open(LOG_FILE_NAME, std::ios::app);
			outFile << data;
			//outFile.close();
		}

		return *this;
	}

	void clear_file()
	{
		if (1 == policy || 2 == policy) {
			std::ofstream outFile;
			outFile.open(LOG_FILE_NAME);
			outFile.close();
		}
	}
public:
	_Out(std::string log_file_name = "log.txt", long policy = 0)
		: LOG_FILE_NAME(log_file_name), policy(policy) {
		
	}
	~_Out() {
		if (outFile.is_open()) {
			outFile.close();
		}
	}

	void SetFileName(const std::string& fileName) {
		LOG_FILE_NAME = fileName;
	}
	void Open() {
		if (1 == policy || 2 == policy) {
			outFile.open(LOG_FILE_NAME);
		}
	}
	void SetPolicy(long policy) {
		this->policy = policy;
	}
};
_Out Out;



std::vector<std::pair<int, std::pair<int/* > 0, < 0 */, std::string>>> diff(clau_parser::UserType* before, clau_parser::UserType* after,
			bool onlyChange = false) {
	std::vector<std::pair<int, std::pair<int/* > 0 , < 0 */, std::string>>> result;

	std::queue<std::pair<int, std::pair<int, std::string>>> plus_result;
	std::queue<std::pair<int, std::pair<int, std::string>>> minus_result;

	std::map<std::string, size_t> before_map; // map -> set
	std::map<std::string, size_t> after_map;

	clau_parser::ClauParserTraverser iter_before(before);
	clau_parser::ClauParserTraverser iter_after(after);

	clau_parser::ClauParserTraverser x = iter_before;
	clau_parser::ClauParserTraverser y = iter_after;
	
	clau_parser::ClauParserTraverser _x = x;
	clau_parser::ClauParserTraverser _y = y;


	clau_parser::ClauParserTraverser __x = _x;
	clau_parser::ClauParserTraverser __y = _y;

	std::string temp;
	std::string str1;
	std::string str2;

	int line = 0;

	while (!x.is_end() && !y.is_end()) {
				
		line++;

		str1.clear();
		str2.clear();

		before_map.clear();
		after_map.clear();

		{
			_x = x;
			_y = y;

			if (_x.get_type() == _y.get_type() && _x.get_string() == _y.get_string()) {


				// not_use?

				result.push_back({ line, { 0, _x.get_string() } });

				x.next();
				y.next();
			}
			else {
				__x = _x;
				__y = _y;

				size_t count__x = 0;
				size_t count__y = 0;

				// find same value position.
				if (__x.get_type() == clau_parser::ValueType::key || __x.get_type() == clau_parser::ValueType::value) {
					before_map.insert({ __x.get_string(), count__x });
				}
				if (__y.get_type() == clau_parser::ValueType::key || __y.get_type() == clau_parser::ValueType::value) {
					after_map.insert({ __y.get_string(), count__y });
				}

				{
					std::string same_value;
					int turn = 1; // 1 -> -1 -> 1 -> -1 ...
					bool pass = false;

					while (!__x.is_end() && !__y.is_end()) {
						if (turn == 1) {
							__x.next();
							count__x++;

							if (__x.get_type() == clau_parser::ValueType::key || __x.get_type() == clau_parser::ValueType::value) {
							}
							else {
								turn *= -1;
								continue;
							}

							before_map.insert({ __x.get_string(), count__x });

							// if found same value?
							if (after_map.end() != after_map.find(__x.get_string())) {
								pass = true;
								same_value = __x.get_string();
								count__y = after_map.find(__x.get_string())->second;
								break;
							}
						}
						else {
							__y.next();
							count__y++;

							if (__x.get_type() == clau_parser::ValueType::key || __x.get_type() == clau_parser::ValueType::value) {
							}
							else {
								turn *= -1;
								continue;
							}

							after_map.insert({ __y.get_string(), count__y });

							// if found same value?
							if (before_map.end() != before_map.find(__y.get_string())) {
								pass = true;
								same_value = __y.get_string();
								count__x = after_map.find(__y.get_string())->second;
								break;
							}
						}

						turn *= -1; // change turn.
					}


					// found same value,
					if (pass) {
						std::string temp;
						
						int _line = line;

						while ((temp = x.get_string()) != same_value) {
							
							if (!plus_result.empty()) {
								if (temp != plus_result.front().second.second) {
									minus_result.push({ _line, { -1, temp } });
								}
								else {
									plus_result.pop();
								}
							}
							else {
								minus_result.push(std::make_pair((_line), std::make_pair(-1, temp)));
							}

							x.next();
						}

						_line = line;

						while ((temp = y.get_string()) != same_value) {
							
							if (!minus_result.empty()) {
								if (temp != minus_result.front().second.second) {
									plus_result.push({ _line, { 1, temp } });
								}
								else {
									minus_result.pop();
								}
							}
							else {
								plus_result.push(std::make_pair((_line), std::make_pair(+1, temp)));
							}

							y.next();
						}
					}
				}
			}
		}
	}

	// delete -
	if (!x.is_end()) {
		std::string temp;

		while (!x.is_end()) {
			temp += x.get_string() + " ";

			x.next();
		}

		++line;

		if (!plus_result.empty()) {
			if (temp != plus_result.front().second.second) {
				minus_result.push({ line, { -1, temp } });
			}
			else {
				plus_result.pop();
			}
		}
		else {
			minus_result.push({ line, { -1, temp } });
		}
	}
	// added +
	if (!y.is_end()) {
		std::string temp;

		while (!y.is_end()) {
			temp += y.get_string() + " ";

			y.next();
		}

		++line;

		if (!minus_result.empty()) {
			if (temp != minus_result.front().second.second) {
				plus_result.push({ line, { 1, temp } });
			}
			else {
				minus_result.pop();
			}
		}
		else {
			plus_result.push({ line, { 1, temp } });
		}
	}

	while (!minus_result.empty()) {
		result.push_back(minus_result.front());
		minus_result.pop();
	}

	while (!plus_result.empty()) {
		result.push_back(plus_result.front());
		plus_result.pop();
	}



	return result;
}

class Comp {
public:
	bool operator()(const std::pair<int /*line */, std::pair<int/* +1, -1 */, std::string>>& x, const std::pair<int/* line */, std::pair<int/* +1, -1 */, std::string>>& y) const {
		if (x.first == y.first) {
			return x.second.first < y.second.first;
		}
		return x.first < y.first;
	}
};

int main(int argc, char* argv[])
{
	if (argc != 3) {
		return 1;
	}

	std::string before_file = argv[1];
	std::string after_file = argv[2];

	clau_parser::UserType beforeUT;
	clau_parser::UserType afterUT;

	// error check.. -todo.
	clau_parser::LoadData::LoadDataFromFile(before_file, beforeUT, 0, 0); // 1 vs 0 - scan.. different algorithms?
	clau_parser::LoadData::LoadDataFromFile(after_file, afterUT, 0, 0);

	Out.SetPolicy(1);
	Out.SetFileName("output.md");
	Out.clear_file();
	Out.Open();


	Out << "```diff" << "\n";

	std::vector<std::pair<int, std::pair<int/* +1, -1 */, std::string>>> result = diff(&beforeUT, &afterUT);


	std::stable_sort(result.begin(), result.end(), Comp());

	int count = 0;
	int state = 0;
	for (size_t i = 0; i < result.size(); ++i) {

		if (i > 0 && result[i - 1].second.first > 0 && result[i].second.first > 0) {
			state = 1;
		}
		else if (i > 0 && result[i - 1].second.first < 0 && result[i].second.first < 0) {
			state = 1;
		}
		else if (i > 0) {

			count++;
			if (count >= 5) {
				Out << "\n";
				count = 0;
			}

			state = 0;
		}
		
		//Out << result[i].first << " ";
		if (result[i].second.first > 0) {
			if (state == 0) {
				Out << "\n+####" << result[i].second.second << " ";
			}
			else {
				Out << result[i].second.second << " ";
			}
		}
		else if (result[i].second.first < 0) {
			if (state == 0) {
				Out << "\n-####" << result[i].second.second << " ";
			}
			else {
				Out << result[i].second.second << " ";
			}
		}
		else {
			Out << result[i].second.second << " ";
		}
	}
	Out << "\n";

	Out << "```" << "\n";

	return 0;
}

