// Copyright (c) 2018-2020  Robert J. Hijmans
//
// This file is part of the "spat" library.
//
// spat is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// spat is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with spat. If not, see <http://www.gnu.org/licenses/>.


#include "spatVector.h"
#include "file_utils.h"

#include "ogrsf_frmts.h"
#include "ogr_spatialref.h"

#include "crs.h"


std::string geomType(OGRLayer *poLayer) {
	std::string s = "";
    poLayer->ResetReading();
    OGRFeature *poFeature;
    while( (poFeature = poLayer->GetNextFeature()) != NULL ) {
		OGRGeometry *poGeometry = poFeature->GetGeometryRef();
		const char* gname = poGeometry->getGeometryName();
		s = gname;
		break;
	}
	OGRFeature::DestroyFeature( poFeature );
	return s;
}


SpatDataFrame readAttributes(OGRLayer *poLayer) {
	SpatDataFrame df;

    OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
	size_t nfields = poFDefn->GetFieldCount();
	if (nfields == 0) return df;

	OGRFieldType ft;
    poLayer->ResetReading();
    OGRFeature *poFeature;
	OGRFieldDefn *poFieldDefn;
	df.resize_cols(nfields);
	bool first = true;
	unsigned dtype;
    while( (poFeature = poLayer->GetNextFeature()) != NULL ) {
		if (first) {
			for (size_t i = 0; i < nfields; i++ ) {
				poFieldDefn = poFDefn->GetFieldDefn(i);
				std::string fname = poFieldDefn->GetNameRef();
				ft = poFieldDefn->GetType();
				if (ft == OFTReal) {
					dtype = 0;
				} else if ((ft == OFTInteger) | (ft == OFTInteger64)) {
					dtype = 1;
				} else {
					dtype = 2;
				}
				df.add_column(dtype, fname);
			}
			first = false;
		}

		for (size_t i = 0; i < nfields; i++ ) {
			poFieldDefn = poFDefn->GetFieldDefn( i );
			unsigned j = df.iplace[i];
			switch( poFieldDefn->GetType() ) {
				case OFTReal:
					df.dv[j].push_back(poFeature->GetFieldAsDouble(i));
					break;
				case OFTInteger:
					df.iv[j].push_back(poFeature->GetFieldAsInteger( i ));
					break;
				case OFTInteger64:
					df.iv[j].push_back(poFeature->GetFieldAsInteger64( i ));
					break;
	//          case OFTString:
				default:
					df.sv[j].push_back(poFeature->GetFieldAsString( i ));
					break;
			}
		}
	}
    OGRFeature::DestroyFeature( poFeature );
	return df;
}


/*
std::string getDs_WKT(GDALDataset *poDataset) { 
	std::string wkt = "";
	char *cp;
#if GDAL_VERSION_MAJOR >= 3
	const OGRSpatialReference *srs = poDataset->GetSpatialRef();
	const char *options[3] = { "MULTILINE=YES", "FORMAT=WKT2", NULL };
	OGRErr err = srs->exportToWkt(&cp, options);
	if (err == OGRERR_NONE) {
		wkt = std::string(cp);
		CPLFree(cp);
	} 
#else
	const char *pszSrc = GDALGetProjectionRef( poDataset );
	if (pszSrc != NULL) { 
		wkt = std::string(pszSrc);
	}
	
//	if (poDataset->GetProjectionRef() != NULL) { 
//		OGRSpatialReference oSRS(poDataset->GetProjectionRef());
//		OGRErr err = oSRS.exportToPrettyWkt(&cp);
//		if (err == OGRERR_NONE) {
//			wkt = std::string(cp);
//			CPLFree(cp);
//		}
//	}
	
#endif 	
	return wkt;
}

std::string getDs_PRJ(GDALDataset *poDataset) { 
	std::string prj = "";
#if GDAL_VERSION_MAJOR >= 3
	char *cp;
	const OGRSpatialReference *srs = poDataset->GetSpatialRef();
	OGRErr err = srs->exportToProj4(&cp);
	if (err == OGRERR_NONE) {
		prj = std::string(cp);
		CPLFree(cp);
	}
#else
	if( poDataset->GetProjectionRef() != NULL ) {
		OGRSpatialReference oSRS(poDataset->GetProjectionRef());
		char *pszPRJ = NULL;
		oSRS.exportToProj4(&pszPRJ);
		prj = pszPRJ;
	}
#endif	
	return prj;
}
*/

bool SpatVector::read_ogr(GDALDataset *poDS) {

	std::string crs = "";
	OGRSpatialReference *poSRS = poDS->GetLayer(0)->GetSpatialRef();
	if (poSRS) {
		char *psz = NULL;
		std::string errmsg;
		OGRErr err = poSRS->exportToWkt(&psz);
		if (is_ogr_error(err, errmsg)) {
			setError(errmsg);
			CPLFree(psz);
			return false;
		}
		crs = psz;
		setSRS(crs);
		CPLFree(psz);
	}

/*
#if GDAL_VERSION_MAJOR >= 3
	OGRSpatialReference *poSRS = poDS->GetLayer(0)->GetSpatialRef();
	std::string wkt="";
	std::string errmsg;
	if (!wkt_from_spatial_reference(poSRS, wkt, errmsg)){
		addWarning(errmsg);
         }
	setSRS(wkt) ; 
	//prj = prj;
#else
	if( poDS->GetProjectionRef() != NULL ) {
		OGRSpatialReference oSRS(poDS->GetProjectionRef());
		char *cp = NULL;
		OGRErr err = oSRS.exportToPrettyWkt(&cp);
		std::string errmsg;
		if (is_ogr_error(err, errmsg)) {
			setError(errmsg);
			CPLFree(cp);
			return false;
		}
		std::string wkt = (std::string) cp;
		setSRS(wkt);
		CPLFree(cp);
	} else {
		addWarning("could not read input SRS");
        }
#endif
*/
/*
	OGRSpatialReference *poSRS = poDS->GetLayer(0)->GetSpatialRef();

	if (poSRS) {
		char *pszCRS = NULL;
		poSRS->exportToProj4(&pszCRS);
		crs = pszCRS;
		prj = pszCRS;

		//pszCRS = NULL;
		//poSRS->exportToWKT(&pszCRS);
		//crs = pszCRS;
	}
*/
	
	//OGRLayer *poLayer = poDS->GetLayerByName( basename_noext(fname).c_str() );
	OGRLayer *poLayer = poDS->GetLayer(0);
	df = readAttributes(poLayer);

	OGRwkbGeometryType wkbgeom = wkbFlatten( poLayer ->GetGeomType());

	OGRFeature *poFeature;
	OGRPoint ogrPt;
	unsigned np, nh, ng;

	poLayer->ResetReading();
	if ((wkbgeom == wkbPoint) | (wkbgeom == wkbMultiPoint)) {
		//SpatPart p(0,0);
		while( (poFeature = poLayer->GetNextFeature()) != NULL ) {
			OGRGeometry *poGeometry = poFeature->GetGeometryRef();
			SpatGeom g(points);
			if (poGeometry != NULL) {
				if ( wkbFlatten(poGeometry->getGeometryType()) == wkbPoint ) {
				#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,3,0)
					OGRPoint *poPoint = poGeometry->toPoint();
				#else
					OGRPoint *poPoint = (OGRPoint *) poGeometry;
				#endif
					double x = poPoint->getX();
					double y = poPoint->getY();
					SpatPart p(x, y);
					g.addPart(p);
				} else {
					OGRMultiPoint *poMultipoint = ( OGRMultiPoint * )poGeometry;
					ng = poMultipoint ->getNumGeometries();
					std::vector<double> X(ng);
					std::vector<double> Y(ng);
					for (size_t i=0; i<ng; i++) {
		              	OGRGeometry *poMpGeometry = poMultipoint->getGeometryRef(i);
					#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,3,0)
						OGRPoint *poPoint = poMpGeometry->toPoint();
					#else
						OGRPoint *poPoint = (OGRPoint *) poMpGeometry;
					#endif
						X[i] = poPoint->getX();
						Y[i] = poPoint->getY();
					}
					SpatPart p(X, Y);
					g.addPart(p);
				}
			} else {
				SpatPart p(NAN, NAN);
				g.addPart(p);
			}
			addGeom(g);
		}
	} else if (wkbgeom == wkbLineString) {
		while ( (poFeature = poLayer->GetNextFeature()) != NULL ) {
			OGRGeometry *poGeometry = poFeature->GetGeometryRef();
			SpatGeom g;
			g.gtype = lines;
			if (poGeometry != NULL) {
				if (wkbFlatten ( poGeometry ->getGeometryType() ) == wkbLineString) {
					OGRLineString *poGeom = (OGRLineString *) poGeometry;
					np = poGeom->getNumPoints();
					std::vector<double> X(np);
					std::vector<double> Y(np);
					for (size_t i=0; i<np; i++) {
						poGeom->getPoint(i, &ogrPt);
						X[i] = ogrPt.getX();
						Y[i] = ogrPt.getY();
					}
					SpatPart p(X, Y);
					g.addPart(p);
				} else {
					OGRMultiLineString *poGeom = ( OGRMultiLineString * )poGeometry;
					ng = poGeom->getNumGeometries();
					for (size_t i=0; i<ng; i++) {
						OGRGeometry *poLineGeometry = poGeom->getGeometryRef(i);
						OGRLineString *poLine = ( OGRLineString * )poLineGeometry;
						np = poLine->getNumPoints();
						std::vector<double> X(np);
						std::vector<double> Y(np);
						for (size_t j=0; j<np; j++ ) {
							poLine->getPoint(j, &ogrPt);
							X[j] = ogrPt.getX();
							Y[j] = ogrPt.getY();
						}
						SpatPart p(X, Y);
						g.addPart(p);
					}
				}
			}
			addGeom(g);
		}
	} else if (wkbgeom == wkbPolygon || wkbgeom == wkbMultiPolygon ) {
		while ( (poFeature = poLayer->GetNextFeature()) != NULL ) {
			OGRGeometry *poGeometry = poFeature ->GetGeometryRef();
			SpatGeom g;
			g.gtype = polygons;
			if (poGeometry != NULL) {
				if ( (poGeometry->getGeometryType() ) == wkbPolygon ) {
					OGRPolygon *poGeom = ( OGRPolygon * )poGeometry;
					OGRLinearRing *poRing = poGeom->getExteriorRing();
					np = poRing->getNumPoints();
					std::vector<double> X(np);
					std::vector<double> Y(np);
					for (size_t i=0; i<np; i++) {
						poRing->getPoint(i, &ogrPt);
						X[i] = ogrPt.getX();
						Y[i] = ogrPt.getY();
					}
					SpatPart p(X, Y);

					nh = poGeom->getNumInteriorRings();
					for (size_t i=0; i<nh; i++) {
						OGRLinearRing *poHole = poGeom->getInteriorRing(i);
						np = poHole->getNumPoints();
						std::vector<double> X(np);
						std::vector<double> Y(np);
						for (size_t j=0; j<np; j++) {
							poHole->getPoint(j, &ogrPt);
							X[j] = ogrPt.getX();
							Y[j] = ogrPt.getY();
						}
						p.addHole(X, Y);
					}
					g.addPart(p);
				} else { //if ( (poGeometry ->getGeometryType()) == wkbMultiPolygon ) {
					OGRMultiPolygon *poGeom = ( OGRMultiPolygon * )poGeometry;
					ng = poGeom->getNumGeometries();
					for (size_t i=0; i<ng; i++) {
						OGRGeometry *poPolygonGeometry = poGeom->getGeometryRef(i);
						OGRPolygon *poPolygon = ( OGRPolygon * )poPolygonGeometry;
						OGRLinearRing *poRing = poPolygon->getExteriorRing();
						np = poRing->getNumPoints();
						std::vector<double> X(np);
						std::vector<double> Y(np);
						for (size_t j=0; j<np; j++ ) {
							poRing->getPoint(j, &ogrPt);
							X[j] = ogrPt.getX();
							Y[j] = ogrPt.getY();
						}
						SpatPart p(X, Y);

						nh = poPolygon->getNumInteriorRings();
						for (size_t j=0; j<nh; j++) {
							OGRLinearRing *poHole = poPolygon->getInteriorRing(j);
							np = poHole->getNumPoints();
							std::vector<double> X(np);
							std::vector<double> Y(np);
							for (size_t k = 0; k < np; k++ ) {
								poHole->getPoint(k, &ogrPt);
								X[k] = ogrPt.getX();
								Y[k] = ogrPt.getY();
							}
							p.addHole(X, Y);
						}
						g.addPart(p);
					}
				}
				addGeom(g);
			}
		}
	} else {
        setError("Cannot open file");
		//std::string gt = geomType(poLayer);
		//printf("unknown geomtype: %s \n", gt.c_str());
		return false;
	}

	OGRFeature::DestroyFeature( poFeature );
 	return true;
}


 // get the extent
 //       OGREnvelope oExt;
 //       if( poLayer->GetExtent( &oExt, true ) == OGRERR_NONE ){
 //           cout << "Extent: (" << oExt.MinX << ", " << oExt.MinY << ") - (" << oExt.MaxX << ", " << oExt.MaxY << ")" << endl;


bool SpatVector::read(std::string fname) {

    //OGRRegisterAll();
    GDALDataset *poDS = static_cast<GDALDataset*>(GDALOpenEx( fname.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL ));
    if( poDS == NULL ) {
        setError("Cannot open this file");
		return false;
    }
	bool success = read_ogr(poDS);
	if (poDS != NULL) GDALClose( poDS );
	return success;
}

SpatVector SpatVector::fromDS(GDALDataset *poDS) {
	SpatVector out;
	out.read_ogr(poDS);
	return out;
}

