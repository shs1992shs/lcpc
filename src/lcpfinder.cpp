/*
 * lcpfinder.cpp
 *
 *  Created on: Aug 25, 2016
 *      Author: elias
 */

#include "lcpfinder.h"
#include <algorithm>
#include <stdexcept>      // std::out_of_range
#include <iomanip>
#include <iostream>

/*
 * Finds opposing by looking up intersection of the immidiate neighbours of either end of the base.
 * This can be either 1 or 2 vertices, of whcih 1 or 0 may be relevant. (irrelevant vertice is already inside funnel)
 * LOOKING UP INTERSECTION REQUIRES SORTING IMMIDIATE NEIGHBOURS. HOW TO MAKE FASTER:
 * ?-> have sorted vector of immidiate neighbours available. (increases insertion time to log N, now constant)
 */


const Coords* LcpFinder::getOpposing(const Coords* l, const Coords* r, int polygon) {
    nContainer intrsct;
    intrsct = intersection(l->getNeighbours(polygon), r->getNeighbours(polygon));
    for (nContainer::iterator it = intrsct.begin(); it != intrsct.end(); it++) {
        if (*it != 0 and (*it)->isRight(l, r) == -1) {
            return *it;
        }
    }

    return 0;
}

void LcpFinder::initQue(std::vector<std::pair<const Coords*, double>>::iterator nit, std::vector<std::pair<const Coords*, double>>::iterator next, const Coords* c, nSet*nset, std::deque<Funnel>* funnelQue) {
    Funnel f(nit->first, c, (next)->first);

    double fric = nit->second;
    auto p = nset->insert(std::pair<const Coords*, double>(nit->first, fric));
    if (!p.second and p.first->second > fric) {
        p.first->second = fric;
    }
    funnelQue->push_back(f);

}

std::deque<Funnel> LcpFinder::initFQues(const Coords* c, int polygon, nSet*nset) {
    std::vector<std::pair<const Coords*, double>>*neigbours = c->getNeighbours(polygon);
    if (!this->triangulated[polygon]) {
        this->triangulate(polygon);
        this->triangulated[polygon] = true;
    }

    std::deque<Funnel> funnelQue{};
    if (c == this->startPoint or c->linePt) {
        this->initQue(neigbours->end() - 1, neigbours->begin(), c, nset, &funnelQue);
    }
    for (std::vector<std::pair<const Coords*, double>>::iterator nit = neigbours->begin(); nit + 1 != neigbours->end(); nit++) {
        this->initQue(nit, nit + 1, c, nset, &funnelQue);
    }

    /*
    std::cout << "BEFORE:\n";
    for (auto it = nset->begin(); it != nset->end(); it++) {
        std::cout << it->first->toString();
        std::cout << it->second << std::endl;
    }*/


    return funnelQue;

}

void LcpFinder::findNeighboursInPolygon(const Coords* c, int polygon, nSet* neighbours) {
    std::deque<Funnel> funnelQue = this->initFQues(c, polygon, neighbours);
    std::pair<const Coords*, const Coords*> base;
    const Coords* o;

    while (!funnelQue.empty()) {
        //std::cout<<"loop..."<<std::endl;
        Funnel f = funnelQue.front();
        funnelQue.pop_front();
        base = f.getBase();
        o = getOpposing(base.first, base.second, polygon);
        if (o != 0) {
            f.reactToOpposite(o, &funnelQue, neighbours, this->frictions[polygon]);
            funnelQue.push_back(f);
        }

    }
}

nSet LcpFinder::findNeighbours(const Coords* c) {

    //std::cout<<"C: "<<c->toString()<<std::endl;
    nSet neighbours{};
    if (c != this->startPoint and c->target) {
        return neighbours;
    }
    std::vector<int> polygons = c->belongsToPolygons();
    for (int p : polygons) {
        if (p != -1) {
            findNeighboursInPolygon(c, p, &neighbours);
        }
    }
    if (c->linePt) {
        std::vector<std::pair <const Coords*, double>>*lineNeighbours = c->getNeighbours(-1);
        for (auto it = lineNeighbours->begin(); it != lineNeighbours->end(); it++) {
            std::pair<nSet::iterator, bool> insert = neighbours.insert(*it);
            if(!insert.second and insert.first->second>it->second){
                insert.first->second = it->second;
            }
        }
    }
    return neighbours;
}

double LcpFinder::toClosestEnd(const Coords* c) {
    double min = std::numeric_limits<double>::max();
    for (std::pair<int, std::vector < p2t::Point*>> polygon : targetPoints) {
        for (p2t::Point* target : polygon.second) {
            double d = eucDistance(target, c);
            if (d < min) {
                min = d;
            }
        }
    }
    return min * this->minFriction;
}

struct cmpr {
    bool (*f)(const Coords*, const Coords*);

    cmpr() {
        f = &compAstar;
    }

    cmpr(bool (*comparefunction)(const Coords*, const Coords*)) {
        f = comparefunction;
    }

    bool operator()(const Coords* x, const Coords* y) const {
        return f(x, y);
    }
};

std::deque<const Coords*> LcpFinder::leastCostPath(int algorithm) {
    if (this->startPoint == 0) {
        std::cout << "No startpoint set in finder. Is the startpoint inside any polygon?\n" << std::endl;
        exit(1);
    }

    int targetsFound = 0;
    this->startPoint->setToStart(0);

    bool (*compareFunction)(const Coords*, const Coords*);


    if (algorithm == 0) {
        compareFunction = &compDijkstra;
    } else if (algorithm == 1) {
        compareFunction = &compAstar;
    }
    cmpr comparator{compareFunction};

    MinHeap<const Coords*, cmpr> minheap(comparator);



    minheap.push(this->startPoint);

    int handled = 0;
    int old = 0;
    int total = this->coordmap.size();

    while (!minheap.empty()) {

        int percentage = handled * 100 / total;
        if (percentage != old) {
            old = percentage;
            std::cout << "\rSearching..." << percentage << "% done";
            fflush(stdout);
        }
        handled++;
        const Coords* node = minheap.top();
        //std::cout<<node->toString()<<std::endl;
        if (node == 0) {
            std::cout << "current node = 0\n";
            exit(2);
        }
        if (node->target) {
            targetsFound++;
            if (targetsFound == this->numOfTargets) {
                break;
            }
        }


        minheap.pop();
        nSet neighbours = findNeighbours(node);
        //std::cout << node->toString() << "# of neighbours: " << neighbours.size() << std::endl;
        for (std::pair<const Coords*, int> p : neighbours) {
            const Coords* n = p.first;
            if (n == 0) {
                std::cout << "neighbour is 0\n";
                exit(2);
            }
            double d{node->getToStart() + eucDistance(node, n) * p.second};
            if (n->getToStart() < 0) { // node has not yet been inserted into minheap
                n->setToStart(d);
                n->setToEnd(this->toClosestEnd(n));
                n->setPred(node);
                minheap.push(n);
            } else if (n->getToStart() > d) {
                n->setToStart(d);
                n->setPred(node);
                minheap.update(); //reorders minheap after changing priority of n
            }
        }
    }
    std::deque<const Coords*> res;
    for (std::pair<int, std::vector < p2t::Point*>> endpoints : this->targetPoints) {
        for (p2t::Point* ep : endpoints.second) {
            if (ep == 0) {
                std::cout << "EP == 0\n";
                continue;
            }
            if (ep->x == this->startPoint->getX() and ep->y == this->startPoint->getY()) {
                continue;
            }
            std::tr1::unordered_set<Coords>::iterator it = this->coordmap.find(Coords(ep->x, ep->y));
            if (it != this->coordmap.end()) {
                res.push_back(&*it);
            }
        }
    }
    return res;
}

void LcpFinder::triangulate(int polygon) {
    std::vector<std::vector < p2t::Point*>> points;
    try {
        points = this->polygons.at(polygon);
    } catch (const std::out_of_range& oor) {
        std::cout << "EXCEPTION WHEN GETTING POLYGON\n";
        std::cout << "polygon:" << polygon << std::endl;
        exit(1);
    }
    p2t::CDT cdt{points[0]}; //Constrained Delaunay triangulator with outer edges.
    for (unsigned int hole = 1; hole < points.size(); hole++) {
        cdt.AddHole(points[hole]);
    }
    bool stp = true;
    std::vector<p2t::Point*> steinerPoints;
    try {
        steinerPoints = this->targetPoints.at(polygon);
    } catch (const std::out_of_range& oor) {
        stp = false;
    }
    if (stp) {
        for (p2t::Point* sp : steinerPoints) {
            cdt.AddPoint(sp);
        }
    }

    bool linepts = true;
    std::vector<p2t::Point*> linepoints;
    try {
        linepoints = this->linePoints.at(polygon);
    } catch (const std::out_of_range& oor) {
        linepts = false;
    }
    if (linepts) {
        for (p2t::Point* lp : linepoints) {
            cdt.AddPoint(lp);
        }
    }

    cdt.Triangulate();
    std::vector<p2t::Triangle*> triangles = cdt.GetTriangles();
    for (std::vector<p2t::Triangle*>::iterator it = triangles.begin(); it != triangles.end(); it++) {
        p2t::Triangle* triangle = *it;
        const Coords * cp[3];
        Coords c[3];
        for (unsigned i = 0; i < 3; i++) {
            p2t::Point* point = triangle->GetPoint(i);
            c[i] = Coords(point->x, point->y);
        }
        // if triangle orientation is clockwise turn it to CCW
        if (c[0].isRight(&c[1], &c[2]) == 1) {
            Coords tmp = c[2];
            c[2] = c[1];
            c[1] = tmp;
        }
        for (unsigned i = 0; i < 3; i++) {
            std::tr1::unordered_set<Coords>::iterator f = coordmap.find(c[i]);
            if (f != coordmap.end()) {
                cp[i] = &*f;
            } else {
            }
        }
        for (unsigned i = 0; i < 3; i++) {
            int l = i - 1;
            int r = i + 1;
            if (l == -1) {
                l = 2;
            }
            if (r == 3) {
                r = 0;
            }

            cp[i]->addNeighbours(cp[l], polygon, this->frictions[polygon]);
            cp[i]->addNeighbours(cp[r], polygon, this->frictions[polygon]);


        }
    }
}

LcpFinder::~LcpFinder() {
    for (std::vector<std::vector < p2t::Point*>> polygon : polygons) {
        for (std::vector<p2t::Point*> ring : polygon) {
            for (p2t::Point* point : ring) {
                delete point;
            }
        }
    }
    for (std::pair<int, std::vector < p2t::Point*>> polygon : targetPoints) {
        for (p2t::Point* point : polygon.second) {
            delete point;
        }
    }

}

void LcpFinder::addPolygon(std::vector<std::vector<p2t::Point*>> points, double friction) {
    //std::cout<<"adding POLYGON\n";
    int polygon = polygons.size();
    if (friction < this->minFriction) {
        minFriction = friction;
    }
    this->frictions.push_back(friction);
    this->triangulated.push_back(false);
    this->polygons.push_back(points);

    for (std::vector<p2t::Point*> ring : points) {
        const Coords* prev = 0;
        const Coords* first = 0;
        for (p2t::Point* point : ring) {
            std::pair < std::tr1::unordered_set<Coords>::iterator, bool> p = coordmap.insert(Coords(point->x, point->y, polygon, false));
            if (!p.second) {
                p.first->addToPolygon(polygon);
            }

            const Coords* cur = &*p.first;
            if (prev != 0) {
                prev->addNeighbours(cur, polygon, friction);
                cur->addNeighbours(prev, polygon, friction);
            } else {
                first = cur;
            }
            prev = &*p.first;

        }
        prev->addNeighbours(first, polygon, friction);
        first->addNeighbours(prev, polygon, friction);
    }
}

/*
void LcpFinder::addLine(std::vector<p2t::Point*>* line, double frictionForwards, double frictionBackwards, std::array<int, 2> polygons) {
    const Coords* prev = 0;
    for (int i = 0; i < line->size(); i++) {
        Coords f{line->at(i)->x, line->at(i)->y};
        auto it = this->coordmap.find(f);
        if (it == this->coordmap.end()) {
            std::cout << f.toString();
            const Coords* closest = 0;
            const Coords* c;
            for (auto it = this->getCoordmap()->begin(); it != this->getCoordmap()->end(); it++) {
                c = &*it;
                if (closest == 0 or eucDistance(c, &f) < eucDistance(closest, &f)) {
                    closest = c;
                }
            }
            std::cout << closest->toString();
            std::cout << "A linear feature's point was not found from cost surface!\n";
            exit(1);
        }
        const Coords* cur = &*it;
        for (int pol : polygons) {
            if (pol > 0 and pol<this->polygons.size()) {
                if (prev != 0) {
                    cur->addNeighbours(prev, pol, frictionBackwards);
                    prev->addNeighbours(cur, pol, frictionForwards);
                }
            }
        }


    }
}
 */
void LcpFinder::addSteinerPoint(p2t::Point* steinerpoint, int polygon) {

    if (this->targetPoints.find(polygon) == this->targetPoints.end()) {
        this->targetPoints[polygon] = std::vector<p2t::Point*>{steinerpoint};
    } else {
        this->targetPoints[polygon].push_back(steinerpoint);
    }
    std::pair < std::tr1::unordered_set<Coords>::iterator, bool> success = this->coordmap.insert(Coords(steinerpoint->x, steinerpoint->y, polygon, true));
    if (!success.second) {
        std::cout << "DID NOT INSERT steinerpoint: " << steinerpoint->x << "," << steinerpoint->y << "into polygon " << polygon << "\n";
        std::cout << success.first->toString() << std::endl;
    } else {
        this->numOfTargets++;
    }
}

const Coords* LcpFinder::addLinePoint(p2t::Point* point, int polygon) {

    if (this->linePoints.find(polygon) == this->linePoints.end()) {
        this->linePoints[polygon] = std::vector<p2t::Point*>{point};
    } else {
        this->linePoints[polygon].push_back(point);
    }
    std::pair < std::tr1::unordered_set<Coords>::iterator, bool> success = this->coordmap.insert(Coords(point->x, point->y, polygon, false, true));
    if (!success.second) {
        success.first->addToPolygon(polygon);
    }
    return &*success.first;
}

void LcpFinder::addLine(std::vector<p2t::Point*>* points, double friction) {
    const Coords* prev = 0;
    for (auto it = points->begin(); it != points->end(); it++) {
        p2t::Point* p = *it;
        std::array<int, 2> polygons = containingPolygon(p);
        const Coords* c = addLinePoint(p, polygons[0]);
        if (polygons[1] != -1) {
            addLinePoint(p, polygons[1]);
        }
        c->addToPolygon(-1);
        if (prev != 0) {
            c->addNeighbours(prev, -1, friction);
            prev->addNeighbours(c, -1, friction);
           
        }
        prev = c;
    }
}

/*
 * Returns at most two polygons inside which a point is.
 */

std::array<int, 2> LcpFinder::containingPolygon(p2t::Point* p) {
    std::array<int, 2>res{-1, -1};
    int found = 0;
    for (int i = 0; i<this->polygons.size(); i++) {
        std::vector<std::vector < p2t::Point*>> polygon = this->polygons[i];
        if (inside(polygon, p)) {
            res[found] = i;
            found++;
            if (found == 2) {
                break;
            }
        }
    }
    return res;
}

void LcpFinder::addStartPoint(p2t::Point* startPoint, int polygon) {
    this->addSteinerPoint(startPoint, polygon);

    this->startPoint = & * this->coordmap.find(Coords(startPoint->x, startPoint->y));

}
