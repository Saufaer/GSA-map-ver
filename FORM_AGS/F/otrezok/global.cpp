#include "global.h"

Pointer global::OptimizeO(const double _left, const double _right, const int _N_max, const double _Eps)
{
	double time = 0;
	Pointer p;
	double curr_left = _left;

	double curr_right = _right;
	p.steps = 0;
	p.time = 0;
	double start, end;

	/////
	double z_begin = funcO(left_xO); // левая граница
	double z_end = funcO(right_xO); // правая граница
	//// прочие переменные
	double temp = 0.; // переменная для подсчёта 
	int k = 1; // количество шагов в Настоящем алгоритме
	int new_size; // тоже шаги
	double M_big, m_small = 1.; // Перменные для хранения итоговой константы "М" и константы "m"
	int R_max_index = 0; // Переменная индекса следующей точки испытания

	double left, right, new_point; // переменные для рабочего отрезка и точки нового испытания
	////
	//// контейнеры для хранения значений и аргументов функции
	std::list<double> z; //значения Z
	std::list<double> x; //значения рабочих X
	std::list<double> M_vector; // вектор неких коэфф M для выбора m = max(M[i])
	std::list<double> R_vector; // вектор значений R для принятия решения точки следующего испытания
	//std::queue<double> M_queue; 
	//std::queue<double> R_queue;


	//// блок итераторов для работы со списками
	std::list<double>::iterator place_x, place_z, place_M, place_R; // итераторы-указатели КУДА положить новые значения
	std::list<double>::iterator iter_x, iter_z, iter_R, iter_M;
	////
	left = curr_left;
	right = curr_right;
	new_point = temp = (curr_left + curr_right) / 2;

	x.push_back(left);
	x.push_back(new_point);
	x.push_back(right);

	iter_x = x.begin();
	for (int i = 0; i < 3; i++)
	{
		z.push_back(funcO(*iter_x++));
	}

	iter_z = z.begin();
	iter_x = x.begin();

	//// начало алгоритма // первый шаг, новая точка - средняя //

	blockO val_z; // структура для хранения левой и правой границы отрезка с Z
	blockO arg_x; // структура для хранения левой и правой границы отрезка с Х

	//Начало времени отсчёта работы алгоритма
	start = omp_get_wtime();

	{
		for (int i = 1; i < 3; i++)
		{
			val_z.leftO = *iter_z++;
			val_z.rightO = *iter_z;
			arg_x.leftO = *iter_x++;
			arg_x.rightO = *iter_x;

			M_vector.push_back(MO(val_z.rightO, val_z.leftO, arg_x.rightO, arg_x.leftO)); // вставка новой М_big
		}

		double max = M_vector.front();

		if (max < M_vector.back()) max = M_vector.back();
		M_big = max; // поиск максимальной M_big

		if (M_big > 0.) m_small = rO*M_big;
		else if (M_big == 0.) m_small = 1.;

		R_max_index = 1;
		max = 0.;

		iter_z = z.begin();
		iter_x = x.begin();
		// считаем и вставляем новую характеристику для рабочего интервала и пересчитываем для получившегося "старого" интервала
		// старый интервал получился делением x(i-1)<x_new<x(i)
		// новый интервал (рабочий) x(i-1)<x_new
		// старый интервал x_new<x(i)
		for (int i = 1; i < 3; i++)
		{
			val_z.leftO = *iter_z++;
			val_z.rightO = *iter_z;
			arg_x.leftO = *iter_x++;
			arg_x.rightO = *iter_x;

			temp = RO(m_small, arg_x.rightO, arg_x.leftO, val_z.rightO, val_z.leftO);
			R_vector.push_back(temp);
			if (i == 1) max = temp;

			if (max < temp)
			{
				max = temp;
				R_max_index = i;
			}
		}

		iter_z = z.begin();
		iter_x = x.begin();

		for (int i = 0; i < R_max_index; i++)
		{
			iter_z++;
			iter_x++;
		}

	
		curr_right = *iter_x;
		curr_left = *--iter_x;

		iter_z = z.begin();
		iter_x = x.begin();
	}

	//// начало работы АГП с испытанием в новой точке по правилу //
	while (k < _N_max && (fabs(curr_left - curr_right) > _Eps))
	{
		new_size = 2 + k; // + 2 для быстрого алгоритма, потом будет корректировка

		place_z = z.begin(); // найдём интервалы с максимальной характеристикой
		place_x = x.begin();
		// ищем интервалы сдвигая итераторы
		for (register int i = 0; i < R_max_index; i++)
		{
			place_z++;
			place_x++;
		}
		// забираем значения для хранения и работы нового цикла
		val_z.rightO = *place_z--;
		val_z.leftO = *place_z++;

		left = curr_left;
		right = curr_right;
		// вычисление новой точки испытания x_new
		new_point = 0.5*(right + left) - (val_z.rightO - val_z.leftO) / (2 * m_small);
		// сохраняем x_new и z_new
		z.insert(place_z, funcO(new_point));
		x.insert(place_x, new_point);

		iter_z = z.begin();
		iter_x = x.begin();
		place_M = M_vector.begin();
		place_R = R_vector.begin();

		// ищем интервал с лучшей характеристикой сдвигом итераторов
		for (int i = 0; i < R_max_index - 1; i++)
		{
			iter_z++;
			iter_x++;
			if (i == R_max_index - 1) break;
			place_M++;
			place_R++;
		}
		// старый интервал получился делением x(i-1)<x_new<x(i)
		// новый интервал (рабочий) x(i-1)<x_new
		// старый интервал x_new<x(i)
		for (int i = 0; i < 2; i++)
		{
			val_z.leftO = *iter_z++;
			val_z.rightO = *iter_z;
			arg_x.leftO = *iter_x++;
			arg_x.rightO = *iter_x;
			if (i == 0)
			{
				M_vector.insert(place_M, MO(val_z.rightO, val_z.leftO, arg_x.rightO, arg_x.leftO)); // x(i - 1)<x_new
			}
			else
			{
				*place_M = MO(val_z.rightO, val_z.leftO, arg_x.rightO, arg_x.leftO); // x_new<x(i)
			}
		}

		iter_M = M_vector.begin();

		double max = *iter_M;
		temp = M_vector.back();

		for (int i = 0; i < new_size - 1 ; i++)
		{
			if (max < temp) max = temp;
			temp = *++iter_M;
		}
		M_big = max;

		if (M_big > 0.) m_small = rO*M_big;
		else if (M_big == 0.) m_small = 1.;

		R_max_index = 1;
		max = 0.;

		--iter_z; --iter_z; // сдвиг интераторов для ускорения
		--iter_x; --iter_x; // обойдёмся без нового поиска, мы помним насколько сдвигали рабочие итераторы 

		// старый интервал получился делением [ x(i-1) < x_new < x(i) ]
		// новый интервал (рабочий) [ x(i-1) < x_new ]
		// старый интервал [ x_new < x(i) ]

		for (int i = 0; i < 2; i++)
		{
			val_z.leftO = *iter_z++;
			val_z.rightO = *iter_z;
			arg_x.leftO = *iter_x++;
			arg_x.rightO = *iter_x;
			if (i == 0)	R_vector.insert(place_R, RO(m_small, arg_x.rightO, arg_x.leftO, val_z.rightO, val_z.leftO)); // x(i - 1)<x_new
			else
				*place_R = RO(m_small, arg_x.rightO, arg_x.leftO, val_z.rightO, val_z.leftO); // x_new<x(i)
		}

		iter_R = R_vector.begin();
		max = *iter_R;
		for (register int i = 1; i < new_size; i++)
		{
			temp = *++iter_R;
			if (max < temp)
			{
				max = temp;
				R_max_index = i + 1; // нашли R(t) // R(i)
			}
		}

		iter_x = x.begin();
		iter_z = z.begin();

		for (register int i = 0; i < R_max_index; i++) // поиск точки глоб.минимума (интервала)
		{
			iter_z++;
			iter_x++;
		}

		// сохранение


		curr_right  = *iter_x;
		curr_left = *--iter_x;
		++k;

		p.X.push_back(curr_right);
		p.steps++;
	}
	p.x = curr_right;
	p.z = *iter_z;

	end = omp_get_wtime();
	time = end - start;
	p.time = time;



		return p;
	
}

Pointer global::ParallelOptimize(const double _left, const double _right, const int _N_max, const double _Eps, const int _threads)
{
	procsO = _threads;

	std::vector<Pointer> optimized_values(procsO); // left = x, right = z;
	omp_set_dynamic(0);

	double start = omp_get_wtime();
	omp_set_num_threads(procsO);


	if (_threads > 1)
	{

#pragma omp parallel for
	for (int i = 0; i < procsO; i++)
	{
		optimized_values[i] = OptimizeO(_left + (_right - _left)*i / procsO, _left + (_right - _left)*(i + 1) / procsO, _N_max, _Eps);

	}

	Pointer res;
	res.z = optimized_values[0].z;
	res.x = optimized_values[0].x;
	res.steps = 0;
	res.time = 0;
	for (int i = 0; i < procsO; i++)
	{
		if (optimized_values[i].z < res.z)
		{
			res.z = optimized_values[i].z;
			res.x = optimized_values[i].x;

		}
		res.time += optimized_values[i].time;
		res.steps += optimized_values[i].steps;
		for (int j = 0; j < optimized_values[i].X.size(); j++)
		{
			res.X.push_back(optimized_values[i].X[j]);
		}

	}
	return res;
}

	
	if (_threads == 1)
	{

			optimized_values[0] = OptimizeO(_left, _right, _N_max, _Eps);
			Pointer res;
			res.z = optimized_values[0].z;
			res.x = optimized_values[0].x;
			res.time = 0;
			res.steps = optimized_values[0].steps;;
			for (int j = 0; j < optimized_values[0].X.size(); j++)
			{

				res.X.push_back(optimized_values[0].X[j]);
			}
			res.time += optimized_values[0].time;
		return res;
	}
	
}

inline double global::RO(const double &_m_small, const double &_x_curr, const double &_x_prev, const double &_z_curr, const double &_z_prev)
{
	return _m_small*(_x_curr - _x_prev) + pow(_z_curr - _z_prev, 2) / (_m_small*(_x_curr - _x_prev)) - 2 * (_z_curr + _z_prev);
}

inline double global::MO(const double &_z_curr, const double &_z_prev, const double &_x_curr, const double &_x_prev)
{
	return fabs((_z_curr - _z_prev) / (_x_curr - _x_prev));
}

inline double global::funcO(const double &_x)
{
	//return sin(_x) + sin((10. * _x) / 3.);
	//return (2 * powl((_x - 3), 2) + exp((powl(_x, 2) / 2)));
	//return ((3 * _x - 1.4)*sin(18 * _x));
	//return (sin(_x) + sin((10. * _x) / 3.) + log(_x) - 0.84*_x + 3);
	//return ((pow(_x, 2) - 5 * _x + 6) / (pow(_x, 2) + 1));
	return 2 * sin(3 * _x) + 3 * cos(5 * _x); // (0,8 r = 2);
}

