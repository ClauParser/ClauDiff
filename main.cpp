
// ClauDiff
#include <iostream>
#include <utility>
#include <vector>
#include <execution> // for parallel sort.

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


	std::string same_value;

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
				if (!onlyChange) {
					result.push_back({ line, { 0, _x.get_string() } });
				}

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
		temp.clear();

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
		temp.clear();
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


class DiffResult {
public:
	bool now_start = false;
	bool now_end = false;
	
	int line = 0;

	clau_parser::ClauParserTraverser x;
	clau_parser::ClauParserTraverser y;

	int type = 0; // -1 delete, +1 add

	std::string str; // key or value.

public:
	DiffResult(int line, const clau_parser::ClauParserTraverser& x, const clau_parser::ClauParserTraverser& y, int type, const std::string& str) 
		: line(line), x(x), y(y), type(type), str(str)
	{
		//
	}

	static DiffResult MakeStart(const clau_parser::ClauParserTraverser& x, const clau_parser::ClauParserTraverser& y) {
		DiffResult temp(-1, x, y, 0, "");
		temp.now_start = true;

		return temp;
	}
	static DiffResult MakeEnd(const clau_parser::ClauParserTraverser& x, const clau_parser::ClauParserTraverser& y) {
		DiffResult temp(-1, x, y, 0, "");
		temp.now_end = true;

		return temp;
	}
};


bool chk = false;

std::vector<DiffResult> diff2(clau_parser::UserType* before, clau_parser::UserType* after,
	bool onlyChange = false) {
	std::vector<DiffResult> result;

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

	clau_parser::ClauParserTraverser temp_x = x;
	clau_parser::ClauParserTraverser temp_y = y;

	clau_parser::ClauParserTraverser temp_ = y;

	clau_parser::ClauParserTraverser before_x = x;
	clau_parser::ClauParserTraverser before_y = y;
	
	std::string temp;
	std::string str1;
	std::string str2;


	std::string same_value;

	int line = 0;
	
	int state = 0;

	while (!x.is_end() && !y.is_end()) {

		str1.clear();
		str2.clear();

		before_map.clear();
		after_map.clear();

		{
			_x = x;
			_y = y;


			if (_x.get_type() == _y.get_type() && _x.get_string() == _y.get_string()) {
				before_x = x;
				before_y = y;
				
				x.next();
				y.next();
				

				continue;
			}


			//if (chk)
			{
			//	std::cout << "chkwww" << x.get_string() << " " << y.get_string() << "\n";
			}

			{
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
					int turn = 1; // 1 -> -1 -> 1 -> -1 ...
					bool pass = false;
					int state = 0;

					while (!__x.is_end() && !__y.is_end()) {
						if (turn == 1) {
							__x.next();
							count__x++;

							if (__x.get_type() == clau_parser::ValueType::key || __x.get_type() == clau_parser::ValueType::value) {
							}
							else {
								state = 1;
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
							else
							{
								state = 2;

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
						if (state == 1) {
							before_x = x;
							x.next();
							
							state = 0;
						}
						else if (state == 2) {
							before_y = y;
							y.next();

							state = 0;
						}

						temp_x = x;
						temp_y = y;

				//		result.push_back(DiffResult::MakeStart(x, y));

						line++;

						int state = 0;
						int state2 = 0;

						while ((temp = x.get_string()) != same_value) {
							if (state2 == 0) {
								if (x.get_type() == clau_parser::ValueType::value) {
									auto name = ((clau_parser::ItemType<std::string>*)x.get_now())->GetName();
									if (!name.empty()) {
										result.emplace_back(line, before_x, temp_y, -1, name);
									}
								}

								state2 = 1;
							}

							state = 0;

							result.emplace_back(line, x, temp_y, -1, temp);
							
							if (x.get_type() == clau_parser::ValueType::key) {
								state = 1;
							}
							
							x.next();
						}

						if (state == 1) {
							temp_ = x;
							
							if (temp_.get_type() == clau_parser::ValueType::value) {
								result.emplace_back(line, x, temp_, -1, x.get_string()); 
					
								x.next();
							}
							state = 0;
						}

						state = 0;
						state2 = 0;
						while ((temp = y.get_string()) != same_value) {
							if (state2 == 0) {
								if (y.get_type() == clau_parser::ValueType::value) {
									auto name = ((clau_parser::ItemType<std::string>*)y.get_now())->GetName();
									if (!name.empty()) {
										result.emplace_back(line, temp_x, before_y, +1, name);
									}
								}

								state2 = 1;
							}

							state = 0;
							result.emplace_back(line, temp_x, y, +1, temp);
							
							if (y.get_type() == clau_parser::ValueType::key) {
								state = 1;
							}


							y.next();
							
						}
						if (state == 1) {
							temp_ = y;

							if (temp_.get_type() == clau_parser::ValueType::value) {
								result.emplace_back(line, y, temp_, +1, y.get_string());

								y.next();
							}
						}
				//		result.push_back(DiffResult::MakeEnd(x, y));
					}
				}
			}
		}
	}

	//result.push_back(DiffResult::MakeStart(x, y));
	++line;

	// delete -
	if (!x.is_end()) {
		temp.clear();

		while (!x.is_end()) {

			result.emplace_back(line, x, y, -1, x.get_string());

			x.next();
		}
	}
	// added +
	if (!y.is_end()) {
		temp.clear();
		while (!y.is_end()) {

			result.emplace_back(line, x, y, +1, y.get_string());

			y.next();
		}
	}

	//result.push_back(DiffResult::MakeEnd(x, y));

	return result;
}


clau_parser::UserType* diff_patch(clau_parser::UserType* ut, std::vector<DiffResult>& diff) {

	if (diff.empty()) {
		return nullptr;
	}

	std::string key;

	clau_parser::Maker maker;
	clau_parser::ClauParserTraverser now(ut);
	clau_parser::ClauParserTraverser temp(ut);

	int count = 0;

	for (size_t i = 0; i < diff.size(); ++i) {
		auto iter = diff[i];
		
		while (now.get_no() < iter.x.get_no()) {

				if (now.get_type() == clau_parser::ValueType::key) {
					key = now.get_string();
				}

				if (now.is_now_it()) {
					if (now.get_type() == clau_parser::ValueType::value) {
						maker.NewItem(std::move((*static_cast<clau_parser::ItemType<std::string>*>(now.get_now()))));
					}
				}
				if (now.get_type() == clau_parser::ValueType::container) {
					maker.NewGroup(key);
					key.clear();
				}

				if (now.get_type() == clau_parser::ValueType::end_of_container) {
					maker.EndGroup();
				}

				now.next();
			}

		std::cout << now.get_string() << " " << (int)now.get_type() << " " << iter.str << " " << iter.type << "\n";

		// REMOVE
		if (iter.type < 0) {

			{
				if (now.get_type() == clau_parser::ValueType::container) {
					count++;
				}
				else if (now.get_type() == clau_parser::ValueType::end_of_container) {
					count--;
				}

				now.next();
			}
		}
		else if (iter.type == 0) {
			// nothing.
		}
		// ADD
		else {

			while (count < 0) {
				maker.EndGroup();
				count++;
			}
			if (iter.y.get_type() == clau_parser::ValueType::key) {
				key = iter.str;
			}

			if (iter.y.get_type() == clau_parser::ValueType::value) {
				maker.NewItem(key, iter.str);
				
				key.clear();
			}
			else if (iter.y.get_type() == clau_parser::ValueType::container) {
				maker.NewGroup(key);
				key.clear();
				
			}

			if (iter.y.get_type() == clau_parser::ValueType::end_of_container) {
				maker.EndGroup();
			}
		}
	}


	while (!now.is_end()) {

		if (now.get_type() == clau_parser::ValueType::key) {
			key = now.get_string();
		}

		if (now.get_type() == clau_parser::ValueType::value) {
			maker.NewItem(std::move((*static_cast<clau_parser::ItemType<std::string>*>(now.get_now()))));
			key.clear();
		}

		if (now.get_type() == clau_parser::ValueType::container) {
			maker.NewGroup(key);
			key.clear();
		}
		

		if (now.get_type() == clau_parser::ValueType::end_of_container) {
			maker.EndGroup();
		}

		now.next();
	}


	return maker.Get();
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
	{
		std::string before_file = argv[1];
		std::string after_file = argv[2];

		clau_parser::UserType beforeUT;
		clau_parser::UserType afterUT;

		// error check.. -todo.
		clau_parser::LoadData::LoadDataFromFile(before_file, beforeUT, 0, 0); // 1 vs 0 - scan.. different algorithms?
		clau_parser::LoadData::LoadDataFromFile(after_file, afterUT, 0, 0);


		auto result = diff2(&beforeUT, &afterUT, false);

		for (auto& x : result) {
			std::cout << x.type << " " << x.str << "\n";
		}

		auto result2 = diff_patch(&beforeUT, result);

		
		{
			std::ofstream file("output.eu4");
			result2->Save1(file);
			file.close();
		}

		
		std::cout << "after patch...\n";

		chk = true;

		result = diff2(result2, &afterUT, false);

		for (auto& x : result) {
			std::cout << x.type << " " << x.str << "\n";
		}

		std::cout << "nothing?\n";

		return 0;
	}

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

	std::vector<std::pair<int, std::pair<int/* +1, -1 */, std::string>>> result = diff(&beforeUT, &afterUT, false);

	// parallel sorting.
	std::stable_sort(std::execution::par, result.begin(), result.end(), Comp());
	
	int chk = 0;
	int count = 0;
	for (size_t i = 0; i < result.size(); ++i) {
		int state = 0;

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
			chk = 1;
		}
		else if (result[i].second.first < 0) {
			if (state == 0) {
				Out << "\n-####" << result[i].second.second << " ";
			}
			else {
				Out << result[i].second.second << " ";
			}
			chk = 1;
		}
		else {
			if (chk) {
				Out << "\n";
				chk = 0;
			}
			Out << result[i].second.second << " ";
		}
	}
	Out << "\n";

	Out << "```" << "\n";

	return 0;
}

