#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>

/* *************************************************************************************************
 *
 *                                   一些辅助类型和辅助函数的定义
 *
 * ************************************************************************************************/

using namespace std;

typedef vector<string> StringArray;

// 查找 vector 里是否有 values
template <class T>
bool contain(const vector<T>& values, const T& value) {
	return std::find(values.begin(), values.end(), value) != values.end();
}

// 查找 map 里是否有 key
template <class K, class V>
bool contain(const map<K, V>& dict, const K& key) {
	return dict.find(key) != dict.end();
}

// 查找 vector 里 values 的索引
template <class T>
int indexOf(const vector<T>& values, const T& value) {
	return std::find(values.begin(), values.end(), value) - values.begin();
}

// 单行字符串分割
template<typename Out>
void splitStringImp(const string& s, char delim, Out result) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) { *(result++) = item; }
}
StringArray splitString(const string& s, char delim) {
	StringArray elems;
	splitStringImp(s, delim, back_inserter(elems));
	return elems;
}

/* *************************************************************************************************
 *
 *                                      地铁站节点类型
 *
 * ************************************************************************************************/

class SubwayNode
{
private:

	string      name_;  // 站名
	StringArray lines_; // 所属的线名

public:

	SubwayNode(const string& name = "")
		: name_(name)
	{ /* nothing needs to do*/
	}

public:

	// 返回站名
	const string& name(void) const {
		return name_;
	}

	// 返回所属全部 line
	const StringArray& lines(void) const {
		return lines_;
	}

public:

	// 设置为在 line 上
	void setOnLine(const string& line) {
		if (!contain(lines_, line))
			lines_.push_back(line);
	}

	// 检查是否在 line 上
	bool isOnLine(const string& line) const {
		return contain(lines_, line);
	}

	// 检查是否为中转站
	bool isTransStation(void) const {
		return lines_.size() > 1;
	}
};

/* *************************************************************************************************
 *
 *                                   行程路线类
 *
 * ************************************************************************************************/

class SubwayPath
{
private:

	StringArray         lines_;
	vector<StringArray> stations_;

public:

	// 默认构建的是一个无效的路线
	SubwayPath(void) {
		/*nothing needs to do*/
	}

	// 构建一个单条线上的（子）路线
	SubwayPath(const string& line, const StringArray& stations) {
		if (!stations.empty()) {
			lines_.push_back(line);
			stations_.push_back(stations);
		}
	}

public:

	// 合并两条路线
	void merge(const SubwayPath& path)
	{
		if (this->isInvalid() | path.isInvalid()) {
			// 任何一条路线无效，合并结果都是无效路线
			lines_.clear();
			stations_.clear();
		}
		else {
			// 两条有效路线才能合并为一个有效路线
			lines_.insert(lines_.end(), path.lines_.begin(), path.lines_.end());
			stations_.insert(stations_.end(), path.stations_.begin(), path.stations_.end());
		}
	}

public:

	// 检查是否为有效路线
	int isInvalid(void) const {
		return lines_.empty();
	}

	// 路线换乘的次数
	int transTimes(void) const {
		if (lines_.empty())
			return 0;
		return lines_.size() - 1;
	}

	// 路线的长度（途径站的个数）
	int length(void) const
	{
		if (lines_.empty())
			return INT32_MAX;

		int length = 1;
		for (int l = 0; l < lines_.size(); ++l)
			length += stations_[l].size() - 1;
		return length;
	}

	// 使用指定输出流输出
	void output(ostream& sout, const string& sep = "\n") const
	{
		if (this->isInvalid()) {
			sout << "ERROR";
			return;
		}

		sout << this->length();
		for (int i = 0; i < stations_[0].size(); ++i)
			sout << sep << stations_[0][i];

		// 存在换乘的情况
		if (stations_.size() > 1) {
			for (int l = 1; l < lines_.size(); ++l)
			{
				// 输出换乘的 iline
				sout << sep << lines_[l];

				// 注意换乘后起点站不输出
				for (int i = 1; i < stations_[l].size(); ++i)
					sout << sep << stations_[l][i];
			}
		}// if ...

	}//void output(...
};


/* *************************************************************************************************
 *
 *                                     地铁网络结构类
 *
 * ************************************************************************************************/

typedef map<string, SubwayNode>    DictNodes;
typedef map<string, StringArray>   DictLines;

struct SubwayNetwork
{
public:

	DictNodes dictNodes_;
	DictLines dictLines_;

public:

	// 构建地铁网络
	SubwayNetwork(const DictLines& dictLines)
	{
		// 保存路线图
		dictLines_ = dictLines;

		// 将所有的站点创建成 SubwayNote 保存到 dictNodes_ 中
		for (auto it = dictLines_.begin(); it != dictLines_.end(); ++it)
		{
			// 检查 dictNodes_ 里是不是已经有了这个 station node，没有就新建
			const StringArray stations = it->second;
			for (int i = 0; i < stations.size(); ++i) {
				if (!contain(dictNodes_, stations[i]))
					dictNodes_[stations[i]] = SubwayNode(stations[i]);
			}
		}

		// 更新 dictNodes_ 里面的 station 和 line 之间的连接关系
		// 注意下面这个循环不能和上面那个进行合并
		for (auto it = dictLines_.begin(); it != dictLines_.end(); ++it) {
			const StringArray stations = it->second;
			for (int i = 0; i < stations.size(); ++i)
				dictNodes_[stations[i]].setOnLine(it->first);
		}
	}

public:

	// 检查地铁网络里是否有 station
	bool hasStation(const string& station) const {
		return contain(dictNodes_, station);
	}

	// 检查地铁网络里是否有 line
	bool hasLine(const string& line) const {
		return contain(dictLines_, line);
	}

public:

	// 得到 stationA 和 stationB 同时处于的 line
	//  1. 注意可能为多个，例如 金钟河大街->肿瘤医院 或 北站->红旗南路
	//  2. 如果不在同一条线上返回空 StringArray
	StringArray getSameLines(const string& stationA, const string& stationB) const
	{
		StringArray sameLines;
		StringArray linesOfA = dictNodes_.at(stationA).lines();
		for (int i = 0; i < linesOfA.size(); ++i) {
			if (dictNodes_.at(stationB).isOnLine(linesOfA[i]))
				sameLines.push_back(linesOfA[i]);
		}
		return sameLines;
	}

	// 得到在 line 上 stationA 和 stationB 之间的站点
	//   1. 如果  stationA 和 stationB 不在 line 上则返回空 StringArray
	StringArray getBetweenStations(const string& stationA, const string& stationB, const string& line) const
	{
		StringArray path;
		const StringArray stations = dictLines_.at(line);
		const int iA = indexOf(stations, stationA);
		const int iB = indexOf(stations, stationB);
		if (iA < iB) {
			for (int i = iA; i <= iB; ++i)
				path.push_back(stations[i]);
		}
		else {
			for (int i = iA; i >= iB; --i)
				path.push_back(stations[i]);
		}
		return path;
	}

	// 得到所有走同一条 line 中 stationA 到 stationB 的最短路线
	//   1. 如果一条线 line 走不通则返回无效的 SubwayPath
	SubwayPath getBestOneLinePath(const string& stationA, const string& stationB) const
	{
		StringArray sameLines = getSameLines(stationA, stationB);

		// 如果走不通则返回无效的 SubwayPath
		if (sameLines.empty())
			return SubwayPath();

		// 走得通就返回最短的 SubwayPath
		SubwayPath bestPath;
		for (int i = 0; i < sameLines.size(); ++i) {
			SubwayPath path = SubwayPath(sameLines[i], getBetweenStations(stationA, stationB, sameLines[i]));
			if (path.length() < bestPath.length())
				bestPath = path;
		}
		return bestPath;
	}

	// 得到 lineA 和 lineB 之间所有的换乘站
	//  1. 注意可能为多个
	//  2. 如果没法换乘就返回空 StringArray
	StringArray getTransStations(const string& lineA, const string& lineB) const
	{
		StringArray transStations;
		const StringArray stationsLineA = dictLines_.at(lineA);
		for (int i = 0; i < stationsLineA.size(); ++i) {
			if (dictNodes_.at(stationsLineA[i]).isOnLine(lineB))
				transStations.push_back(stationsLineA[i]);
		}
		return transStations;
	}

	// 换乘方案结构
	struct TransPlan { string lineA, stationT, lineB; };

	// 得到所有从 stationA 到 station B 之间只换乘一次的换乘方案
	//   1. 如果走不通则返回空 vector<TransPlan>
	vector<TransPlan> getTransPlans(const string& stationA, const string& stationB) const
	{
		vector<TransPlan> transPlans;
		const StringArray lineAs = dictNodes_.at(stationA).lines();
		const StringArray lineBs = dictNodes_.at(stationB).lines();

		for (int iA = 0; iA < lineAs.size(); ++iA) {
			for (int iB = 0; iB < lineBs.size(); ++iB) {
				if (lineAs[iA] == lineBs[iB]) continue;
				StringArray stationTs = getTransStations(lineAs[iA], lineBs[iB]);
				if (!stationTs.empty()) { // 没有中转站就算了
					for (int iT = 0; iT < stationTs.size(); ++iT) {
						TransPlan plan;
						plan.lineA = lineAs[iA];
						plan.stationT = stationTs[iT];
						plan.lineB = lineBs[iB];
						transPlans.push_back(plan);
					}
				} // if (..
			}//for (int iB
		}//for (int iA
		return transPlans;
	}

	// 得到从 stationA 到 station B 只换乘一次路线中的最短路线
	//   1. 如果走不通则返回无效的 SubwayPath
	SubwayPath getBestTwoLinesPath(const string& stationA, const string& stationB) const
	{
		vector<TransPlan> transPlans = getTransPlans(stationA, stationB);

		// 如果走不通则返回无效的 SubwayPath
		if (transPlans.empty())
			return SubwayPath();

		// 走得通就返回最短的 SubwayPath
		SubwayPath bestPath;
		for (int i = 0; i < transPlans.size(); ++i) {
			SubwayPath path = getBestOneLinePath(stationA, transPlans[i].stationT);
			path.merge(getBestOneLinePath(transPlans[i].stationT, stationB));
			if (path.length() < bestPath.length())
				bestPath = path;
		}
		return bestPath;
	}

	// 得到从 stationA 到 station B 的最短简单路线
	//   1. 如果走不通则返回无效的 SubwayPath
	SubwayPath getBestSimplePath(const string& stationA, const string& stationB) const
	{
		SubwayPath bestPath;
		const SubwayPath pathOneLine = getBestOneLinePath(stationA, stationB);
		if (pathOneLine.length() < bestPath.length())
			bestPath = pathOneLine;
		const SubwayPath pathTwoLines = getBestTwoLinesPath(stationA, stationB);
		if (pathTwoLines.length() < bestPath.length())
			bestPath = pathTwoLines;
		return bestPath;
	}


	// 找 station 所有相邻的中转站
	StringArray getNearbyTransStations(const string& station) const
	{
		const StringArray lines = dictNodes_.at(station).lines();

		// 挨个 line 的找相邻的中转站
		StringArray transStations;
		for (int l = 0; l < lines.size(); ++l)
		{
			// 找到当前站点在改线上的位置
			const StringArray stations = dictLines_.at(lines[l]);
			const int ipos = indexOf(stations, station);

			// 向前找最近的中转站
			for (int i = ipos + 1; i < stations.size(); ++i) {
				if (dictNodes_.at(stations[i]).isTransStation()) {
					transStations.push_back(stations[i]);
					break;
				}
			}

			// 向后找最近的中转站
			for (int i = ipos - 1; i >= 0; --i) {
				if (dictNodes_.at(stations[i]).isTransStation()) {
					transStations.push_back(stations[i]);
					break;
				}
			}
		}

		return transStations;
	}

	// 递归调用查找所有 A 到 AT 到 B 中最短路线
	SubwayPath getRecursivePath1(const string& stationA, const string& stationB) const
	{
		// 先确定是否能走简单路线
		SubwayPath bestPath = getBestSimplePath(stationA, stationB);
		if (!bestPath.isInvalid())
			return bestPath;

		// 递归调用查找所有 A 到 AT 到 B 中最短路线
		StringArray stationATs = getNearbyTransStations(stationA);
		for (int iA = 0; iA < stationATs.size(); ++iA) {
			SubwayPath path = getBestOneLinePath(stationA, stationATs[iA]); // A 到 AT
			path.merge(getRecursivePath1(stationATs[iA], stationB));        // AT 到 B，注意此处递归了
			if (path.length() < bestPath.length())
				bestPath = path;
		}

		return bestPath;
	}

	// 递归调用查找所有 A 到 BT 到 B 的最短路线
	SubwayPath getRecursivePath2(const string& stationA, const string& stationB) const
	{
		// 先确定是否能走简单路线
		SubwayPath bestPath = getBestSimplePath(stationA, stationB);
		if (!bestPath.isInvalid())
			return bestPath;

		// 递归调用查找所有 A 到 BT 到 B 的最短路线
		StringArray stationBTs = getNearbyTransStations(stationB);
		for (int iB = 0; iB < stationBTs.size(); ++iB) {
			SubwayPath path = getRecursivePath2(stationA, stationBTs[iB]);  // A 到 BT，注意此处递归了
			path.merge(getBestOneLinePath(stationBTs[iB], stationB));       // BT 到 B，注意此处递归了
			if (path.length() < bestPath.length())
				bestPath = path;
		}

		return bestPath;
	}

	// 递归调用查找所有 A 到 AT 到 BT 到 B 的路线
	SubwayPath getRecursivePath3(const string& stationA, const string& stationB) const
	{
		// 先确定是否能走简单路线
		SubwayPath bestPath = getBestSimplePath(stationA, stationB);
		if (!bestPath.isInvalid())
			return bestPath;

		// 递归调用查找所有 A 到 AT 到 BT 到 B 的路线
		StringArray stationATs = getNearbyTransStations(stationA);
		StringArray stationBTs = getNearbyTransStations(stationB);
		for (int iA = 0; iA < stationATs.size(); ++iA) {
			for (int iB = 0; iB < stationBTs.size(); ++iB) {
				SubwayPath path = getBestOneLinePath(stationA, stationATs[iA]); // A 到 AT
				path.merge(getRecursivePath3(stationATs[iA], stationBTs[iB]));  // AT 到 BT 注意此处递归了
				path.merge(getBestOneLinePath(stationBTs[iB], stationB));       // BT 到 A
				if (path.length() < bestPath.length())
					bestPath = path;
			}// for (int iA...
		}// for (int iB...

		return bestPath;
	}

	// 找 A 和 B 之间的最佳路线
	SubwayPath getBestPath(const string& stationA, const string& stationB) const
	{
		SubwayPath bestPath;

		// 三种递归算法选择路线都试一遍，选最短的
		const SubwayPath path1 = getRecursivePath1(stationA, stationB);
		if (path1.length() < bestPath.length())
			bestPath = path1;

		const SubwayPath path2 = getRecursivePath2(stationA, stationB);
		if (path2.length() < bestPath.length())
			bestPath = path2;

		const SubwayPath path3 = getRecursivePath3(stationA, stationB);
		if (path3.length() < bestPath.length())
			bestPath = path3;

		return bestPath;
	}
};

/* *************************************************************************************************
 *
 *                                           实现功能的函数
 *
 * ************************************************************************************************/

 //
 // 帮助文档字符串
 //
const char* HELP_DOC = "\nlist stations of LINE:\n\n"
"  > subway.exe -a LINE -map INPUT.txt -o OUTPUT.txt\n\n"
"best path from STATION_A to STATION_B:\n\n"
"  > subway.exe -b STATION_A STATION_B -map INPUT.txt -o OUTPUT.txt\n\n";

//
// 命令行参数解析结果
//
struct Arguments
{
public:

	string process;     // 操作的类型
	string inputPath;   // 输入文件的路径
	string outputPath;  // 输出文件的路径
	string parameters[2];  // 操作的参数

public:

	Arguments(void)
		: process("default")
	{ /* nothing needs to do */
	}
};

//
// 解析命令行指令参数
//
Arguments ProcessParseArguments(int argc, char** argv)
{
	StringArray argArray(argc);
	for (int i = 0; i < argArray.size(); ++i)
		argArray[i] = argv[i];

	Arguments args;
	for (int i = 0; i < argArray.size(); ++i) {
		if (argArray[i] == "-o") // 输出数据文件路线
			args.outputPath = argArray[i + 1];
		if (argArray[i] == "-map") // 输入数据文件路线
			args.inputPath = argArray[i + 1];
		if (argArray[i] == "-a") { // 显示站点
			args.process = "-a";
			args.parameters[0] = argArray[i + 1];
		}
		if (argArray[i] == "-b") { // 查找 A -> B 的路线
			args.process = "-b";
			args.parameters[0] = argArray[i + 1];
			args.parameters[1] = argArray[i + 2];
		}
	}

	return args;
}

//
// 解析输入数据文件
//
DictLines ProcessParseInputFile(const string& inputPath)
{
	// 先逐行读进来
	StringArray lines;
	ifstream fin;
	try {
		fin.open(inputPath, ios::in);
		while (!fin.eof()) {
			string buf;
			fin >> buf;
			lines.push_back(buf);
		}
	}
	catch (...) {
		fin.close();
		cerr << "ERROR: read input file \"" << inputPath << "\"failed !" << endl;
		return DictLines();
	}


	// 逐行解析
	DictLines dictLines;
	for (int i = 0; i < lines.size(); ++i) {
		StringArray words = splitString(lines[i], ',');
		if (words.size() > 1) {
			string lineName = words[0];
			words.erase(words.begin());
			dictLines[lineName] = words;
		}
	}
	return dictLines;
}

//
// 赠送功能：所有站到站之间路线的计算
//
int ProcessDefault(const Arguments& args)
{
	// 检查命令行指令中输入输出文件是否指定
	if (args.inputPath.empty()) {
		cerr << "ERROR: the path of input data file is unknown !" << endl << HELP_DOC;

		return EXIT_FAILURE;
	}
	else if (args.outputPath.empty()) {
		cerr << "ERROR: the path of output result file is unknown !" << endl << HELP_DOC;
		return EXIT_FAILURE;
	}

	// 读入数据文件，构建地铁路线网
	SubwayNetwork subway(ProcessParseInputFile(args.inputPath));

	// 得到所有的站
	StringArray stations;
	for (auto it = subway.dictNodes_.begin(); it != subway.dictNodes_.end(); ++it)
		stations.push_back(it->first);

	// 输出到输出文件
	ofstream fout;
	try {
		fout.open(args.outputPath, ios::out);
		for (int iA = 0; iA < stations.size(); ++iA) {
			for (int iB = 0; iB < stations.size(); ++iB) {
				if (iA == iB) continue;
				SubwayPath bestPath = subway.getBestPath(stations[iA], stations[iB]);
				fout << "[" << stations[iA] << "->" << stations[iB] << ", ";
				fout << bestPath.transTimes() << " 次中转]: ";
				bestPath.output(fout, ", ");
				fout << endl;
			}
		}
	}
	catch (...) {
		fout.close();
		cerr << "ERROR: write output file \"" << args.outputPath << "\"failed !" << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

//
// 输出指定线的全部站点
//
int ProcessOutputLineStations(const Arguments& args)
{
	// 检查命令行指令中输入输出文件是否指定
	if (args.inputPath.empty()) {
		cerr << "ERROR: the path of input data file is unknown !" << endl << HELP_DOC;
		return EXIT_FAILURE;
	}
	else if (args.outputPath.empty()) {
		cerr << "ERROR: the path of output result file is unknown !" << endl << HELP_DOC;
		return EXIT_FAILURE;
	}

	// 只用读入数据文件并解析，不需要构建地铁网络
	DictLines dictLines = ProcessParseInputFile(args.inputPath);

	// 检查有没有这个 line
	const string line = args.parameters[0];
	if (contain(dictLines, line)) {
		cerr << "ERROR: No line with name \"" << line << "\" !\n" << endl;
		return EXIT_FAILURE;
	}

	// 输出指定线的站点到输出文件
	ofstream fout;
	try {
		fout.open(args.outputPath, ios::out);
		const StringArray stations = dictLines.at(line);
		for (int i = 0; i < stations.size(); ++i)
			fout << stations[i] << endl;
	}
	catch (...) {
		fout.close();
		cerr << "ERROR: write output file \"" << args.outputPath << "\"failed !" << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

//
// 输出 A 到 B 的最佳路线
//
int ProcessOutputBestPath(const Arguments& args)
{
	// 检查命令行指令中输入输出文件是否指定
	if (args.inputPath.empty()) {
		cerr << "ERROR: the path of input data file is unknown !" << endl << HELP_DOC;
		return EXIT_FAILURE;
	}
	else if (args.outputPath.empty()) {
		cerr << "ERROR: the path of output result file is unknown !" << endl << HELP_DOC;
		return EXIT_FAILURE;
	}

	// 读入数据文件，构建地铁路线网
	SubwayNetwork subway(ProcessParseInputFile(args.inputPath));

	// 检查站点输入是否正确
	const string stationA = args.parameters[0];
	const string stationB = args.parameters[1];
	if (!subway.hasStation(stationA)) {
		cerr << "ERROR: No station with name \"" << stationA << "\" !\n" << endl;
		return EXIT_FAILURE;
	}
	if (!subway.hasStation(stationB)) {
		cerr << "ERROR: No station with name \"" << stationB << "\" !\n" << endl;
		return EXIT_FAILURE;
	}

	// 找最佳的路线
	SubwayPath bestPath = subway.getBestPath(stationA, stationB);

	// 输出查找结果到输出文件
	ofstream fout;
	try {
		fout.open(args.outputPath, ios::out);
		bestPath.output(fout);
	}
	catch (...) {
		fout.close();
		cerr << "ERROR: write output file \"" << args.outputPath << "\"failed !" << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/* *************************************************************************************************
 *
 *                                       主函数
 *
 * ************************************************************************************************/

// #define TEST /* 不测试就注释掉 */

#ifdef TEST
void TEST_BEST_PATH(const string& stationA, const string& stationB)
{
	SubwayNetwork subway(ProcessParseInputFile("subway.txt"));

	cout << endl;
	SubwayPath bestPath = subway.getBestPath(stationA, stationB);

	cout << "[" << stationA << "->" << stationB << "]" << endl
		<< "  转乘次数: " << bestPath.transTimes() << endl
		<< "  最短路线: ";
	bestPath.output(cout, ", ");
	cout << endl;
}
#endif

int main(int argc, char** argv)
{
#ifdef TEST
	TEST_BEST_PATH("洪湖里", "复兴门");
	TEST_BEST_PATH("洪湖里", "靖江路");
	TEST_BEST_PATH("洪湖里", "太湖路");
	TEST_BEST_PATH("鞍山道", "东海路");
	TEST_BEST_PATH("南站", "东海路");
	return EXIT_SUCCESS;
#endif

	Arguments args = ProcessParseArguments(argc, argv);
	if (args.process == "-a")
		return ProcessOutputLineStations(args);
	else if (args.process == "-b")
		return ProcessOutputBestPath(args);
	else
		return ProcessDefault(args);
	return EXIT_SUCCESS;
}
