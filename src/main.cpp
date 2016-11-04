//============================================================================
// Name        : lcpc.cpp
// Author      : Elias Annila
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include "coords.h"
#include "funnel.h"
#include "min_heap.h"
#include "lcpfinder.h"
#include "geomfunc.h"
#include "../lib/poly2tri.h"
#include <vector>
#include <gdal/gdal.h>
#include <gdal/ogrsf_frmts.h>
#include <ctime>
#include <string.h>
#include<iostream>

void savePolygon(std::vector<std::vector<p2t::Point*>> p2tp, int i) {
    std::cout << "savePolygon\n";

    std::cout << p2tp[0].size() << std::endl;
    std::cout << i << std::endl;

    OGRSFDriver *driver;
    OGRRegisterAll();
    driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
    OGRDataSource *pointDS;

    std::string outputpolygon = "outpolygon" + std::to_string(i) + ".shp";

    std::cout << outputpolygon << std::endl;

    pointDS = driver->CreateDataSource(outputpolygon.c_str(), NULL);


    if (pointDS == NULL) {
        printf("Creation of output file failed.\n");
        exit(1);
    }
    OGRLayer *pointLayer;
    OGRSpatialReference sr;
    sr.importFromEPSG(3067);

    pointLayer = pointDS->CreateLayer("polygon", &sr, wkbPolygon, NULL);

    if (pointLayer == NULL) {
        printf("Point layer creation failed.\n");
        exit(1);
    }

    OGRFeature *poFeature;
    poFeature = OGRFeature::CreateFeature(pointLayer->GetLayerDefn());

    OGRPolygon poly;

    for (std::vector<p2t::Point*> ring : p2tp) {
        OGRLinearRing lr;

        for (p2t::Point* pt : ring) {
            lr.addPoint(pt->x, pt->y);
        }
        poly.addRing(&lr);
    }


    poFeature->SetGeometry(&poly);
    if (pointLayer->CreateFeature(poFeature) != OGRERR_NONE) {
        printf("Failed to create feature in shapefile.\n");
        exit(1);
    }
    std::cout << "hello2\n";
    OGRFeature::DestroyFeature(poFeature);

    OGRDataSource::DestroyDataSource(pointDS);
    std::cout << "done saving\n";
}

bool comparePolys(OGRPolygon* ogr, std::vector<std::vector<p2t::Point*>> p2tp) {
    if (ogr->getExteriorRing()->getNumPoints() != p2tp[0].size()) {
        std::cout << "ext ring size: " << ogr->getExteriorRing()->getNumPoints() << " vs. " << p2tp[0].size() << std::endl;
        return false;
    }

    if (ogr->getNumInteriorRings() != p2tp.size() - 1) {
        std::cout << "num of int rings: " << ogr->getNumInteriorRings() << " vs. " << p2tp.size() - 1 << std::endl;
        return false;
    }
    for (int i = 0; i < p2tp.size(); i++) {
        if (ogr->getInteriorRing(i)->getNumPoints() != p2tp[i + 1].size()) {
            std::cout << "int ring #" << i + 1 << " ring size: " << ogr->getInteriorRing(i)->getNumPoints() << " vs. " << p2tp[i + 1].size() << std::endl;
            return false;
        }
    }
    return true;

}

void checkCRS(OGRLayer* csLr, OGRLayer* targetLr, OGRLayer* startLr) {
    if (!csLr->GetSpatialRef()->IsProjected()) {
        std::cout << "All source files must be in same projected coordinate system. Geocentric coordinate systems are not supported.\n";
        exit(1);
    }

    int csEpsg = csLr->GetSpatialRef()->GetEPSGGeogCS();
    int targetEpsg = csLr->GetSpatialRef()->GetEPSGGeogCS();
    int startEpsg = csLr->GetSpatialRef()->GetEPSGGeogCS();

    if (csEpsg == -1) {
        std::cout << "WARNING: Unknown cost surface spatial reference EPSG.\n";
    }
    if (targetEpsg == -1) {
        std::cout << "WARNING: Unknown target layer spatial reference EPSG.\n";
    }
    if (startEpsg == -1) {
        std::cout << "WARNING: Unknown start layer spatial reference EPSG.\n";
    }
    if (csEpsg != targetEpsg or csEpsg != startEpsg) {

        std::cout << "ERROR: Cost surface CRS EPSG number doesn't match that of start or target layers. All source files must be in same EPSG projection\n";
        exit(1);
    }


}

void readCostSurface(const char* costSurface, const char* targets, const char* start, LcpFinder* finder, const char* frictionField, std::string maxDist, OGRSpatialReference* sr) {
    double maxd = atof(maxDist.c_str());
    OGRDataSource *csDS;
    csDS = OGRSFDriverRegistrar::Open(costSurface);
    if (csDS == NULL) {
        std::cout << costSurface;
        std::cout << " COST SURFACE NOT FOUND!\n";
        return;
    }
    OGRDataSource *targetDS;
    targetDS = OGRSFDriverRegistrar::Open(targets);
    if (targetDS == NULL) {
        std::cout << "TARGET POINTS NOT FOUND!\n";
        return;
    }
    OGRDataSource *startDS;
    startDS = OGRSFDriverRegistrar::Open(start);
    if (startDS == NULL) {
        std::cout << "START POINT NOT FOUND!\n";
        return;
    }

    OGRLayer* csLr = csDS->GetLayer(0);
    OGRLayer* targetLr = targetDS->GetLayer(0);
    OGRLayer* startLr = startDS->GetLayer(0);


    checkCRS(csLr, targetLr, startLr);
    *sr = *(csLr->GetSpatialRef());

    std::cout << csLr->GetFeatureCount() << " cost surface features found" << std::endl;
    std::cout << targetLr->GetFeatureCount() << " target points found" << std::endl;
    std::cout << startLr->GetFeatureCount() << " start point found" << std::endl;


    OGRFeature *startFtre = startLr->GetNextFeature();

    std::vector<OGRPoint*> targetOGRPoints;

    std::vector<OGRFeature*> targetPointers;
    OGRFeature * targetFtre;
    while ((targetFtre = targetLr->GetNextFeature()) != NULL) {
        targetPointers.push_back(targetFtre);
        OGRGeometry* targetGeom = targetFtre->GetGeometryRef();
        if (targetGeom != NULL
                && wkbFlatten(targetGeom->getGeometryType()) == wkbPoint) {
            OGRPoint* targetPoint = (OGRPoint*) targetGeom;
            targetOGRPoints.push_back(targetPoint);
        } else {
            printf("no target geometry\n");
        }

    }


    OGRPoint* startPoint;
    p2t::Point* startp2t;
    OGRGeometry* startGeom = startFtre->GetGeometryRef();

    if (startGeom != NULL
            && wkbFlatten(startGeom->getGeometryType()) == wkbPoint) {
        startPoint = (OGRPoint*) startGeom;
        startp2t = new p2t::Point(startPoint->getX(), startPoint->getY());
    } else {
        printf("no start geometry\n");
    }


    int pIdx = 0;
    OGRFeature * csFtre;
    while ((csFtre = csLr->GetNextFeature()) != NULL) {
        OGRGeometry* csGeometry = csFtre->GetGeometryRef();
        if (csGeometry != NULL
                && wkbFlatten(csGeometry->getGeometryType()) == wkbPolygon) {
            OGRPolygon *csPolygon = (OGRPolygon *) csGeometry;

            OGRLinearRing* extRing = csPolygon->getExteriorRing();
            if (extRing->isClockwise()) {
                extRing->reverseWindingOrder();
            }

            for (unsigned int ri = 0; ri < csPolygon->getNumInteriorRings(); ri++) {
                OGRLinearRing* intRing = csPolygon->getInteriorRing(ri);
                if (intRing->isClockwise()) {
                    intRing->reverseWindingOrder();
                }
            }

            std::vector<std::vector<std::vector<p2t::Point*> > > sPolygons = simplify(csPolygon);

            for (std::vector<std::vector<p2t::Point*> > polygon : sPolygons) {
                if (maxd > 0) {
                    intermidiatePoints(&polygon, maxd);
                }
                finder->addPolygon(polygon, csFtre->GetFieldAsDouble(frictionField));

                if (inside(polygon, startp2t)) {
                    finder->addStartPoint(startp2t, pIdx);
                }
                for (int i = 0; i < targetOGRPoints.size(); i++) {
                    p2t::Point* targetp2t = new p2t::Point(targetOGRPoints[i]->getX(), targetOGRPoints[i]->getY());
                    if (inside(polygon, targetp2t)) {
                        finder->addSteinerPoint(targetp2t, pIdx);
                    } else {
                        delete targetp2t;
                    }
                }
                pIdx++;
            }


        } else {
            printf("no polygon geometry\n");
        }
        OGRFeature::DestroyFeature(csFtre);

    }

    for (OGRFeature* targetPt : targetPointers) {

        OGRFeature::DestroyFeature(targetPt);

    }

    OGRFeature::DestroyFeature(startFtre);



    OGRDataSource::DestroyDataSource(csDS);
    OGRDataSource::DestroyDataSource(startDS);
    OGRDataSource::DestroyDataSource(targetDS);


}

bool fileExists(const std::string& name) {
    if (FILE * file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {

        return false;
    }
}

bool testDriver(OGRSFDriver* driver) {
    if (driver == NULL) {
        printf("Driver not available Do you want to use default driver ESRI Shapefile?.\n");
        std::string useShp;
        std::cin >> useShp;
        bool isYes = false;
        std::vector<std::string> yesOptions = {"yes", "Yes", "YES", "y", "Y"};
        for (std::string yes : yesOptions) {
            if (useShp == yes) {

                return false;
                break;
            }
        }
        exit(1);
    }
    return true;
}

std::string validateOutfile(OGRSFDriver* driver, std::string outputfile) {

    if (fileExists(outputfile)) {
        std::cout << "Filename already in use, do you want to overwrite? (yes/Yes/YES/Y/y for yes, anything else for no)\n";
        std::string overwrite;
        std::cin >> overwrite;
        bool ow = false;
        std::vector<std::string> yesOptions = {"yes", "Yes", "YES", "y", "Y"};
        for (std::string yes : yesOptions) {
            if (overwrite == yes) {
                driver->DeleteDataSource((outputfile).c_str());
                ow = true;
                break;
            }
        }
        if (!ow) {

            std::cout << "Give new outputfilename that is not in use: \n";
            std::cin >> outputfile;
        }
    }
    return outputfile;
}

void writePoints(std::deque<const Coords*> results, std::string outputfile, std::string pszDriverName, OGRSpatialReference sr, bool overwrite) {
    OGRSFDriver *driver;
    OGRRegisterAll();
    driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(pszDriverName.c_str());

    if (!overwrite) {
        if (!testDriver(driver)) {
            driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
        }
    }


    OGRDataSource *pointDS;

    if (!overwrite) {
        outputfile = validateOutfile(driver, outputfile);
    } else if (fileExists(outputfile)) {
        driver->DeleteDataSource((outputfile).c_str());
    }
    pointDS = driver->CreateDataSource((outputfile).c_str(), NULL);
    if (pointDS == NULL) {
        printf("Creation of output file failed.\n");
        exit(1);
    }


    OGRLayer *pointLayer;
    pointLayer = pointDS->CreateLayer("point_results", &sr, wkbPoint, NULL);
    if (pointLayer == NULL) {
        printf("Point layer creation failed.\n");
        exit(1);
    }

    OGRFieldDefn cField("cost", OFTReal);
    cField.SetPrecision(2);


    if (pointLayer->CreateField(&cField) != OGRERR_NONE) {
        printf("Creating Cost field failed.\n");
        exit(1);
    }

    for (const Coords* point : results) {
        OGRFeature *poFeature;

        poFeature = OGRFeature::CreateFeature(pointLayer->GetLayerDefn());
        poFeature->SetField("cost", point->getToStart());
        OGRPoint ogrpt(point->getX(), point->getY());
        poFeature->SetGeometry(&ogrpt);

        if (pointLayer->CreateFeature(poFeature) != OGRERR_NONE) {

            printf("Failed to create feature in shapefile.\n");
            exit(1);
        }

        OGRFeature::DestroyFeature(poFeature);
    }

    OGRDataSource::DestroyDataSource(pointDS);


}

void writeOutput(std::deque<const Coords*> results, std::string outputfile, std::string pszDriverName, OGRSpatialReference sr, bool overwrite) {

    OGRSFDriver *driver;
    OGRRegisterAll();
    driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(pszDriverName.c_str());
    if (!overwrite) {
        if (!testDriver(driver)) {
            driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
        }
    }
    OGRDataSource *pathDS;
    if (!overwrite) {
        outputfile = validateOutfile(driver, outputfile);

    } else if (fileExists(outputfile)) {
        driver->DeleteDataSource((outputfile).c_str());
    }
    pathDS = driver->CreateDataSource((outputfile).c_str(), NULL);
    if (pathDS == NULL) {
        printf("Creation of output file failed.\n");
        exit(1);
    }


    OGRLayer *pathLayer;
    pathLayer = pathDS->CreateLayer("least_cost_path", &sr, wkbLineString, NULL);
    if (pathLayer == NULL) {
        printf("Path layer creation failed.\n");
        exit(1);
    }

    OGRFieldDefn cField("cost_end", OFTReal);
    cField.SetPrecision(2);


    OGRFieldDefn xField("x", OFTReal);
    xField.SetPrecision(2);

    OGRFieldDefn yField("y", OFTReal);
    yField.SetPrecision(2);


    if (pathLayer->CreateField(&cField) != OGRERR_NONE) {
        printf("Creating Cost field failed.\n");
        exit(1);
    }
    if (pathLayer->CreateField(&xField) != OGRERR_NONE) {
        printf("Creating Cost field failed.\n");
        exit(1);
    }
    if (pathLayer->CreateField(&yField) != OGRERR_NONE) {

        printf("Creating Cost field failed.\n");
        exit(1);
    }



    std::map<const Coords*, const Coords*> ancestors;
    std::map<const Coords*, OGRLineString*> geom;

    struct compToStart {

        bool operator()(const Coords* x, const Coords* y) const {
            return ((x->getToStart()) < (y->getToStart()));
        }
    } compare{};
    MinHeap<const Coords*, compToStart> minheap(compare);



    for (const Coords* point : results) {



        minheap.push(point);
        ancestors[point] = point;
        geom[point] = new OGRLineString{};
        geom[point]->addPoint(point->getX(), point->getY());
    }

    while (!minheap.empty()) {
        const Coords* point = minheap.top();
        const Coords* pred = point->getPred();
        minheap.pop();
        if (pred == 0) {
            continue;
        }

        const Coords* ancestor = ancestors[point];

        if (ancestors.find(pred) != ancestors.end() and pred->getPred() != 0) {
            ancestors[pred] = pred;
            geom[pred] = new OGRLineString{};
            geom[pred]->addPoint(pred->getX(), pred->getY());
        } else {
            ancestors[pred] = ancestor;
            minheap.push(pred);
        }
        geom[ancestor]->addPoint(pred->getX(), pred->getY());

    }

    for (std::pair<const Coords*, OGRLineString*> ls : geom) {
        OGRFeature *poFeature;

        poFeature = OGRFeature::CreateFeature(pathLayer->GetLayerDefn());
        poFeature->SetField("cost_end", ls.first->getToStart());
        poFeature->SetField("x", ls.first->getX());
        poFeature->SetField("y", ls.first->getY());


        poFeature->SetGeometry(ls.second);

        if (pathLayer->CreateFeature(poFeature) != OGRERR_NONE) {

            printf("Failed to create feature in shapefile.\n");
            exit(1);
        }

        OGRFeature::DestroyFeature(poFeature);
        delete ls.second;
    }

    OGRDataSource::DestroyDataSource(pathDS);

}

bool argExists(std::string argSwitch, char* argv[], int argc) {
    for (int i = 0; i < argc; i++) {
        if (argSwitch.compare(argv[i]) == 0) {
            return true;
        }
    }
    return false;
}

std::string getArgVal(std::string argSwitch, char* argv[], int argc) {
    for (int i = 0; i < argc; i++) {
        if (argSwitch.compare(argv[i]) == 0) {
            return argv[i + 1];
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2 or strcmp(argv[1], "-h") == 0) {
        std::cout << "This program is used to search for least cost paths in polygonal costsurface for more information see: URL.\n";
        std::cout << "USAGE:\n"
                "The program requires at least 4 parameters to run correctly. Parameters should be in following order:\n"
                "\tname of cost surface file. (Supports formats supported by OGR)\n"
                "\tname of target points file can be any number of points. All points must be inside cost surface polygons. (Supports formats supported by OGR)\n"
                "\tname of start point file, must be single point inside one of the cost surface polygons. (Supports formats supported by OGR)\n"
                "\tname of the friction field in cost surface\n"
                "Addittionally following parameters can be specified with formt <switch> <value> (for example -o out_path.shp):\n"
                "\t -o output_path_file_name, if no extension is given a folder will be assumed (at least with shapefile driver)\n"
                "\t -p output_points_file_name, if no extension is given a folder will be assumed (at least with shapefile driver)\n"
                "\t -d maximum_distance_between_nodes in cost surface. Default value is 0 (no nodes added). This is used to add temporary additional nodes during LCP calculation if nodes are too far apart.\n"
                "\t --driver output_driver_name. Default value is ESRI Shapefile (Supports drivers supported by OGR)\n"
                "\t -a algorithm to use in lcp search. Astar is default value. Options are dijksra/astar. Using astar is recommended unless number of targetpoints is high compared to number of nodes."
                "\t --overwrite if used will not prompt on anything and just overwrite all files";
        exit(0);
    }

    /*
    std::cout << argc << " provided:";
    for (int i = 1; i < argc; i++) {
        std::cout << (i) << ": " << argv[i] << std::endl;
    }
     */

    if (argc < 5) {
        std::cout << "Invalid arguments provided see \"lcp -h\" for details\n";
        return 0;
    }
    int alg = 1;
    if (argExists("-a", argv, argc)) {
        std::string algArg = getArgVal("-a", argv, argc);
        if (algArg.compare("astar") == 0) {
            alg = 1;
        } else if (algArg.compare("dijkstra") == 0) {
            alg = 0;
        } else {
            std::cout << "unknown search algorithm. using A* (possible choises are \"astar\" or \"dijkstra\"\n";
        }
    }



    bool overwrite = argExists("--overwrite", argv, argc);

    std::string distance = "0";
    if (argExists("-d", argv, argc)) {
        distance = getArgVal("-d", argv, argc);
    }

    OGRRegisterAll();
    LcpFinder finder{};
    std::cout << "Reading cost surface...\n";
    OGRSpatialReference sr;
    std::clock_t begin = std::clock();

    readCostSurface(argv[1], argv[2], argv[3], &finder, argv[4], distance, &sr);
    double secs = double(std::clock() - begin) / CLOCKS_PER_SEC;
    std::cout << "Finished reading cost surface (took " << secs << " s). Starting LCP search...\n";
    begin = std::clock();
    std::deque<const Coords*> results = finder.leastCostPath(alg);
    secs = double(std::clock() - begin) / CLOCKS_PER_SEC;
    std::cout << "\nSearch finished (took " << secs << "s). Writing results...\n";

    std::string driver = "ESRI Shapefile";
    if (argExists("--driver", argv, argc)) {
        driver = getArgVal("--driver", argv, argc);
    }

    if (argExists("-o", argv, argc)) {
        writeOutput(results, getArgVal("-o", argv, argc), driver, sr, overwrite);
    }
    if (argExists("-p", argv, argc)) {
        writePoints(results, getArgVal("-p", argv, argc), driver, sr, overwrite);
    }
    std::cout << "All done!\n";
    return 0;
}