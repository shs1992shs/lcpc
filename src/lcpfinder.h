/*
 * lcpfinder.h
 *
 *  Created on: Aug 25, 2016
 *      Author: elias
 */

#ifndef SRC_LCPFINDER_H_
#define SRC_LCPFINDER_H_
#include <algorithm>
#include <iostream>
#include "Coords.h"
#include "Funnel.h"
#include <tr1/functional>
#include <tr1/unordered_set>

struct CoordsHasher {
	std::size_t operator()(const Coords& c) const {
		std::size_t res = 17;
		res = res * 31 + std::tr1::hash<double>()(c.getX());
		res = res * 31 + std::tr1::hash<double>()(c.getY());
		return res;
	}
};
class LcpFinder{
private:
	std::tr1::unordered_set<Coords, CoordsHasher> coordmap;

	const Coords* getOpposing(const Coords* l, const Coords* r, int polygon);
	std::deque<Funnel> initFQue(const Coords* c, int polygon, std::set<const Coords*>*neighbours);
	void findNeighboursInPolygon(const Coords* c, int polygon, nSet* neighbours);
	nSet findNeighbours(const Coords* c);
public:
	void leastCostPath(const Coords* start, const Coords* end);
	void addPolygon(int polygon, std::vector<std::vector<Coords>> points);

};



#endif /* SRC_LCPFINDER_H_ */