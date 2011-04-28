/***************************************************************************
 *            nge_dirent.hpp
 *
 *  2011/03/25 08:30:03
 *  Copyright  2011  Kun Wang <ifreedom.cn@gmail.com>
 ****************************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _NGE_DIRENT_HPP
#define _NGE_DIRENT_HPP

#if defined __linux__ || defined IPHONEOS
#error "Don't support linux and iphone now"
#endif
#include <vector>
#include <stack>
#include <string>
#include <iostream>

using namespace std;

typedef enum
{
	IS_DIR = 0, /*目录*/
	IS_FILE 	/*文件==Unknow*/
} file_type;

/*
创建时间
修改时间
*/
typedef struct
{
	int year;
	int month;
	int day;

	int hour;
	int min;
	int sec;
}file_time;



class DIR
{
private:
	string name;	/*文件名: 用于显示*/
	string path;	/*文件绝对路径 电脑是全文件名 PSP是短文件名:用于访问*/
	int size;		/*文件大小:byte*/
	file_type type;	/*文件类型 [目录|文件]*/

	file_time ctime; /*创建时间*/
	file_time mtime; /*修改时间*/

public:

	DIR()
	{


	}

	DIR(string n,string p,int s,file_type t,file_time ct,file_time mt)
	{
		name=n;
		path=p;
		size=s;
		type=t;

		ctime=ct;
		mtime=mt;
	}

	string getPath()
	{
		return path;
	}

	string getName() const
	{
		return name;
	}

	int getSize() const
	{
		return size;
	}

	file_type getType() const
	{
		return type;
	}

	file_time getCreatTime() const
	{
		return ctime;
	}

	file_time getModityTime() const
	{
		return mtime;
	}
};

class DIRComparatorByName
{
public:
	bool operator()(const DIR& d1,const DIR& d2) const
	{
		return d1.getName() < d2.getName();
	}
};

class DIRComparatorByType
{
public:
	bool operator()(const DIR& d1,const DIR& d2) const
	{
		return d1.getType()<d2.getType();
	}
};

class DIRComparatorBySize
{
public:
	bool operator()(const DIR& d1,const DIR& d2) const
	{
		return d1.getSize()<d2.getSize();
	}
};

class DIRComparatorByCreatTime
{
public:
	bool operator()(const DIR& d1,const DIR& d2) const
	{
		int year =d1.getCreatTime().year-d2.getCreatTime().year;
		int month=d1.getCreatTime().month-d2.getCreatTime().month;
		int day  =d1.getCreatTime().day-d2.getCreatTime().day;

		if(year<0)
		{
			return true;
		}
		else if(year>0)
		{
			return false;
		}
		else
		{
			if(month<0)
			{
				return true;
			}
			else if(month>0)
			{
				return false;
			}
			else
			{
				if(day<0)
				{
					return true;
				}
				else if(day>0)
				{
					return false;
				}
			}
		}

		return false;
	}
};


class DIRComparatorByModityTime
{
public:
	bool operator()(const DIR& d1,const DIR& d2) const
	{
		int year =d1.getModityTime().year-d2.getModityTime().year;
		int month=d1.getModityTime().month-d2.getModityTime().month;
		int day  =d1.getModityTime().day-d2.getModityTime().day;

		if(year<0)
		{
			return true;
		}
		else if(year>0)
		{
			return false;
		}
		else
		{
			if(month<0)
			{
				return true;
			}
			else if(month>0)
			{
				return false;
			}
			else
			{
				if(day<0)
				{
					return true;
				}
				else if(day>0)
				{
					return false;
				}
			}
		}
		return false;
	}
};

class DIR_Sta
{
private:
	string path;
public:
	string getPath()
	{
		return path;
	}
};

class NGEDirent
{
public:
	NGEDirent();
	~NGEDirent();

	//通过类返回
	/**[中文单层遍历]--构建单层目录索引,资源管理器
	 * 目录遍历访问函数,支持中、英、日路径和中、英、日文件名.
	 *  基于 8.3 短文件名 (UTF-8) 理论可以支持任何编码文件名
	 * 注意在PSP上 文件名 大小写不会变化,访问不分大小写.
	 *@param string directory, 待访问目录名
	 *@param vector<DIR> &dirVec, 访问回调目录vector
	 ** <vector> dirVec 动态分配空间，
	 ** 访问类似数组下标，同时也支持迭代器遍历
	 ** dirVec.begin() == dirVec[0]
	 ** 目录文件总数 = dirVec.size()
	 *@return int ,返回值-1(搜索失败), 文件数
	 */
	int openDirent(string directory);

	//直接返回目录结构
	int openDirent(string directory,vector<DIR> &dirVec);

	//[中文深层遍历]--构建多层目录索引,得到目录文件总大小
	int openDeepDirent(string directory);
	int openDeepDirent(string directory,vector<DIR> &dirVec);

	//关闭目录
	void closeDirent();

	//获取目录
	vector<DIR> getDirent()
	{
		return vec;
	}

	//获取当前搜索目录
	string getCurrentPath()
	{
		return currentPath;
	}

	/*综合排序
	* 排序 总排序规则--文件夹优先
	* 先按 文件类型排序[文件夹>文件]
	* 后按 文件名排序
	*
	* [DIR0]
	* [DIR1]
	* [FILE0]
	* [FILE1]
	*/
	void sortDirent();

	//按文件名排序--不考虑文件类型
	void sortDirentByName();

	//按文件类型排序
	void sortDirentByType();

	//按文件大小排序
	void sortDirentBySize();

	//按创建时间排序
	void sortDirentByCreatTime();

	//按修改时间排序
	void sortDirentByModityTime();


private:

	vector<DIR> vec;

	string currentPath;
};

#endif /* _NGE_DIRENT_HPP */
