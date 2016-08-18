/*
 * Coords.cpp
 *
 *  Created on: Aug 16, 2016
 *      Author: elias
 */

#include "Coords.h"
#include <sstream>
#include <math.h>

Coords::Coords(double newx, double newy) {
	x = newx;
	y = newy;
}

std::string Coords::toString(){
	std::stringstream sstm;
	sstm <<"X: "<<x<<" Y: "<<y;
	return  sstm.str();
}
int Coords::isRight(Coords* c1, Coords* c2){
	double d = (c2->getY() - c1->getY()) * (x - c2->getX())
	                - (c2->getX() - c1->getX()) * (y - c2->getY());
	 if(d<0){
	 		 return -1;
	 }if(d>0){
		 return 1;
	 }
	 return 0;
}

double Coords::eucDistSquared(Coords* c1) {
	return pow(c1->getX()-x, 2) + pow(c1->getY()-y,2);
}

double Coords::eucDist(Coords* c1){
	return sqrt(this->eucDistSquared(c1));
}

Coords::~Coords() {
	// TODO Auto-generated destructor stub
}

