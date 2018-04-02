#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <stdio.h>
#include <vector>
#include <queue>
#include <list>

#include <omp.h>

#include <math.h>

struct blockO
{
	double leftO;
	double rightO;
};
struct Pointer
{
	double x, z;
	int steps;
	std::vector<double> X;
	double time;
};

class global
{
private:
	double left_xO; // левая граница
	double right_xO; // правая граница
	double rO; // параметр метода
	int procsO ;
	
	
	

public:
	
	global(const double _left, const double _right, const double _r) { left_xO = _left, right_xO = _right, rO = _r; }; // использовать данный конструктор
	global() { left_xO = 0; right_xO = 0; rO = 0; }; // по-умолч
	Pointer OptimizeO(const double _left, const double _right, const int _N_max, const double _Eps); // последовательный АГП с возвратом x_min
	Pointer ParallelOptimize(const double _left, const double _right, const int _N_max, const double _Eps, const int _threads); // параллельный по отрезку АГП с возвратом x_min
	inline double RO(const double &_m_small, const double &_z_curr, const double &_z_prev, const double &_x_curr, const double &_x_prev);
	inline double MO(const double &_z_curr, const double &_z_prev, const double &_x_curr, const double &_x_prev);
	inline double funcO(const double &_x);
	

	
};

#endif