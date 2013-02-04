/*
  Copyright 2011 SINTEF ICT, Applied Mathematics.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPM_BOUNDARYPERIODICITY_HEADER_INCLUDED
#define OPM_BOUNDARYPERIODICITY_HEADER_INCLUDED


#include <dune/common/fvector.hh>
#include <dune/common/array.hh>
#include <opm/core/utility/ErrorMacros.hpp>

#include <algorithm>
#include <vector>

namespace Opm
{

    /// @brief
    /// @todo Doc me!
    struct BoundaryFaceInfo
    {
	/// Face index in [0, ..., #faces - 1]
	int face_index;
        /// Boundary id of this face.
	int bid;
        /// Canonical position of face:
        ///  0 -> xmin
        ///  1 -> xmax
        ///  2 -> ymin
        ///  3 -> ymax
        /// ...
	int canon_pos;
        /// Face index of periodic partner, or -1 if no partner.
	int partner_face_index;
        /// Boundary id of periodic partner face, or 0 if no parner.
	int partner_bid;
        /// Face area.
	double area;
        /// Face centroid.
	Dune::FieldVector<double,3> centroid;

        /// Comparison operator.
        /// Intended to make periodic partners appear close in
        /// sorted order, but this is only a heuristic.
	bool operator<(const BoundaryFaceInfo& other) const
	{
	    return cmpval() < other.cmpval();
	}

    private:
	double cmpval() const
	{
            const double pi = 3.14159265358979323846264338327950288;
	    return centroid[(canon_pos/2 + 1)%3] + pi*centroid[(canon_pos/2 + 2)%3];
	}
    };


    /// @brief Find a match (periodic partner) for the given face.
    /// @param[inout] bfaces the boundary face info list.
    /// @param[in] face the face for which we seek a periodic partner
    /// @param[in] lower lower end of search interval [lower, upper)
    /// @param[in] upper upper end of search interval [lower, upper)
    /// @return true if a match was found, otherwise false
    bool match(std::vector<BoundaryFaceInfo>& bfaces, int face, int lower, int upper)
    {
	const double area_tol = 1e-6;
	const double centroid_tol = 1e-6;
	int cp = bfaces[face].canon_pos;
	int target_cp = (cp%2 == 0) ? cp + 1 : cp - 1;
	Dune::FieldVector<double, 3> cent_this = bfaces[face].centroid;
	for (int j = lower; j < upper; ++j) {
	    if (bfaces[j].canon_pos == target_cp) {
		if (fabs(bfaces[face].area - bfaces[j].area) <= area_tol) {
		    Dune::FieldVector<double, 3> cent_other = bfaces[j].centroid;
		    cent_other -= cent_this;
		    double dist = cent_other.two_norm();
		    if (dist <= centroid_tol) {
			bfaces[face].partner_face_index = bfaces[j].face_index;
			bfaces[face].partner_bid = bfaces[j].bid;
			bfaces[j].partner_face_index = bfaces[face].face_index;
			bfaces[j].partner_bid = bfaces[face].bid;
			break;
		    }
		}
	    }
	}
	return (bfaces[face].partner_face_index != -1);
    }



    /// @brief Common implementation for the various createPeriodic functions.
    template <class GridView>
    void findPeriodicPartners(std::vector<BoundaryFaceInfo>& bfinfo,
                              Dune::array<double, 6>& side_areas,
                              const GridView& g,
                              const Dune::array<bool, 2*GridView::dimension>& is_periodic,
                              double spatial_tolerance = 1e-6)
    {
	// Pick out all boundary faces, simultaneously find the
	// bounding box of their centroids, and the max id.
        const int dim = GridView::dimension;
        typedef typename GridView::template Codim<0>::Iterator CI;
	typedef typename GridView::IntersectionIterator FI;
	typedef Dune::FieldVector<double, dim> Vector;
	std::vector<FI> bface_iters;
	Vector low(1e100);
	Vector hi(-1e100);
	int max_bid = 0;
	for (CI c = g.template begin<0>(); c != g.template end<0>(); ++c) {
	    for (FI f = g.ibegin(*c); f != g.iend(*c); ++f) {
		if (f->boundaryId()) {
		    bface_iters.push_back(f);
		    Vector fcent = f->geometry().center();
		    for (int dd = 0; dd < dim; ++dd) {
			low[dd] = std::min(low[dd], fcent[dd]);
			hi[dd] = std::max(hi[dd], fcent[dd]);
		    }
		    max_bid = std::max(max_bid, f->boundaryId());
		}
	    }
	}
	int num_bdy = bface_iters.size();
	if (max_bid != num_bdy) {
	    THROW("createPeriodic() assumes that every boundary face has a unique boundary id. That seems to be violated.");
	}

	// Store boundary face info in a suitable structure. Also find side total volumes.
	std::fill(side_areas.begin(), side_areas.end(), 0.0);
	bfinfo.clear();
	bfinfo.reserve(num_bdy);
	for (int i = 0; i < num_bdy; ++i) {
	    BoundaryFaceInfo bf;
	    bf.face_index = i;
	    bf.bid = bface_iters[i]->boundaryId();
	    bf.canon_pos = -1;
	    bf.partner_face_index = -1;
	    bf.partner_bid = 0;
	    bf.area = bface_iters[i]->geometry().volume();
	    bf.centroid = bface_iters[i]->geometry().center();
	    for (int dd = 0; dd < dim; ++dd) {
		double coord = bf.centroid[dd];
		if (fabs(coord - low[dd]) <= spatial_tolerance) {
		    bf.canon_pos = 2*dd;
		    break;
		} else if (fabs(coord - hi[dd]) <= spatial_tolerance) {
		    bf.canon_pos = 2*dd + 1;
		    break;
		}
	    }
	    if (bf.canon_pos == -1) {
		std::cerr << "Centroid: " << bf.centroid << "\n";
		std::cerr << "Bounding box min: " << low << "\n";
		std::cerr << "Bounding box max: " << hi << "\n";
		THROW("Boundary face centroid not on bounding box. Maybe the grid is not an axis-aligned shoe-box?");
	    }
	    side_areas[bf.canon_pos] += bf.area;
	    bf.centroid[bf.canon_pos/2] = 0.0;
	    bfinfo.push_back(bf);
	}
	ASSERT(bfinfo.size() == bface_iters.size());

	// Sort the infos so that partners end up close.
	std::sort(bfinfo.begin(), bfinfo.end());

	// Identify partners.
	for (int i = 0; i < num_bdy; ++i) {
	    if (bfinfo[i].partner_face_index != -1) {
		continue;
	    }
	    if (!is_periodic[bfinfo[i].canon_pos]) {
		continue;
	    }
	    int lower = std::max(0, i - 10);
	    int upper = std::min(num_bdy, i + 10);
	    bool ok = match(bfinfo, i, lower, upper);
	    if (!ok) {
		// We have not found a partner.
		ok = match(bfinfo, i, 0, num_bdy);
		if (!ok) {
		    MESSAGE("Warning: No partner found for boundary id " << bfinfo[i].bid);
		    // THROW("No partner found.");
		}
	    }
	}
    }

    /*
    /// @brief Common implementation for the various createPeriodic functions.
    template <class GridInterface>
    void findPeriodicPartners(std::vector<BoundaryFaceInfo>& bfinfo,
                              Dune::array<double, 6>& side_areas,
                              const GridInterface& g,
                              const Dune::array<bool, 2*GridInterface::Dimension>& is_periodic,
                              double spatial_tolerance = 1e-6)
    {
	// Pick out all boundary faces, simultaneously find the
	// bounding box of their centroids, and the max id.
	typedef typename GridInterface::CellIterator CI;
	typedef typename CI::FaceIterator FI;
	typedef typename GridInterface::Vector Vector;
	std::vector<FI> bface_iters;
	Vector low(1e100);
	Vector hi(-1e100);
	int max_bid = 0;
	for (CI c = g.cellbegin(); c != g.cellend(); ++c) {
	    for (FI f = c->facebegin(); f != c->faceend(); ++f) {
		if (f->boundaryId()) {
		    bface_iters.push_back(f);
		    Vector fcent = f->centroid();
		    for (int dd = 0; dd < GridInterface::Dimension; ++dd) {
			low[dd] = std::min(low[dd], fcent[dd]);
			hi[dd] = std::max(hi[dd], fcent[dd]);
		    }
		    max_bid = std::max(max_bid, f->boundaryId());
		}
	    }
	}
	int num_bdy = bface_iters.size();
	if (max_bid != num_bdy) {
	    THROW("createPeriodic() assumes that every boundary face has a unique boundary id. That seems to be violated.");
	}

	// Store boundary face info in a suitable structure. Also find side total volumes.
	std::fill(side_areas.begin(), side_areas.end(), 0.0);
	bfinfo.clear();
	bfinfo.reserve(num_bdy);
	for (int i = 0; i < num_bdy; ++i) {
	    BoundaryFaceInfo bf;
	    bf.face_index = i;
	    bf.bid = bface_iters[i]->boundaryId();
	    bf.canon_pos = -1;
	    bf.partner_face_index = -1;
	    bf.partner_bid = 0;
	    bf.area = bface_iters[i]->area();
	    bf.centroid = bface_iters[i]->centroid();
	    for (int dd = 0; dd < GridInterface::Dimension; ++dd) {
		double coord = bf.centroid[dd];
		if (fabs(coord - low[dd]) <= spatial_tolerance) {
		    bf.canon_pos = 2*dd;
		    break;
		} else if (fabs(coord - hi[dd]) <= spatial_tolerance) {
		    bf.canon_pos = 2*dd + 1;
		    break;
		}
	    }
	    if (bf.canon_pos == -1) {
		std::cerr << "Centroid: " << bf.centroid << "\n";
		std::cerr << "Bounding box min: " << low << "\n";
		std::cerr << "Bounding box max: " << hi << "\n";
		THROW("Boundary face centroid not on bounding box. Maybe the grid is not an axis-aligned shoe-box?");
	    }
	    side_areas[bf.canon_pos] += bf.area;
	    bf.centroid[bf.canon_pos/2] = 0.0;
	    bfinfo.push_back(bf);
	}
	ASSERT(bfinfo.size() == bface_iters.size());

	// Sort the infos so that partners end up close.
	std::sort(bfinfo.begin(), bfinfo.end());

	// Identify partners.
	for (int i = 0; i < num_bdy; ++i) {
	    if (bfinfo[i].partner_face_index != -1) {
		continue;
	    }
	    if (!is_periodic[bfinfo[i].canon_pos]) {
		continue;
	    }
	    int lower = std::max(0, i - 10);
	    int upper = std::min(num_bdy, i + 10);
	    bool ok = match(bfinfo, i, lower, upper);
	    if (!ok) {
		// We have not found a partner.
		ok = match(bfinfo, i, 0, num_bdy);
		if (!ok) {
		    MESSAGE("Warning: No partner found for boundary id " << bfinfo[i].bid);
		    // THROW("No partner found.");
		}
	    }
	}
    }
*/



} // namespace Opm

#endif // OPM_BOUNDARYPERIODICITY_HEADER_INCLUDED
