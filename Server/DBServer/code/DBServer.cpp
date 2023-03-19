// TestWindows.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "AppManager.h"
#include "json.hpp"
#include <fstream>
#include "UserData.h"
#include <vector>
#include <time.h>
#include <queue>
#include <stack>

using Json = nlohmann::json;
using namespace nlohmann::detail;
using namespace std;

void replace_all(string& src, const string& old_value, const string& new_value) 
{
	// npos可以表示string的结束位子，是string::type_size 类型的，也就是find（）返回的类型。
	// find函数在找不到指定值得情况下会返回string::npos。
	// 每次重新定位起始位置，防止上轮替换后的字符串形成新的old_value
// 	for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) 
// 	{
// 		if ((pos = src.find(old_value, pos)) != string::npos)
// 		{
// 			src.replace(pos, old_value.length(), new_value);
// 		}
// 		else break;
// 	}
	// 和上面的是等价的
	// string::npos代表string的最后，这还挺方便的
	for (int pos = 0; pos != string::npos; pos += new_value.length())
	{
		if ((pos = src.find(old_value, pos)) != string::npos)
		{
			src.replace(pos, old_value.length(), new_value);
		}
		else break;
	}
}



int main()
{
 	app::run();

}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
