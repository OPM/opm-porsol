//===========================================================================
//
// File: NonuniformTableLinear.hpp
//
// Created: Tue Oct 21 13:25:34 2008
//
// Author(s): Atgeirr F Rasmussen <atgeirr@sintef.no>
//
// $Date$
//
// $Revision$
//
//===========================================================================

/*
  Copyright 2009, 2010 SINTEF ICT, Applied Mathematics.
  Copyright 2009, 2010 Statoil ASA.

  This file is part of The Open Reservoir Simulator Project (OpenRS).

  OpenRS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OpenRS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OpenRS.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPENRS_NONUNIFORMTABLELINEAR_HEADER
#define OPENRS_NONUNIFORMTABLELINEAR_HEADER

#include <cmath>
#include <exception>
#include <vector>
#include <utility>

#include <dune/common/ErrorMacros.hpp>
#include <dune/solvers/common/linearInterpolation.hpp>

namespace Dune {
    namespace utils {


	/// @brief Exception used for domain errors.
	struct OutsideDomainException : public std::exception {};


	/// @brief This class uses linear interpolation to compute the value
	///        (and its derivative) of a function f sampled at possibly
	///         nonuniform points.
	/// @tparam T the range type of the function (should be an algebraic ring type)
	template<typename T>
	class NonuniformTableLinear
	{
	public:
	    /// @brief Default constructor.
	    NonuniformTableLinear();

	    /// @brief Useful constructor.
	    /// @param x_values vector of domain values
	    /// @param y_values vector of corresponding range values.
	    NonuniformTableLinear(const std::vector<double>& x_values,
				  const std::vector<T>& y_values);

	    /// @brief Get the domain.
	    /// @return the domain as a pair of doubles.
	    std::pair<double, double> domain();

	    /// @brief Rescale the domain.
	    /// @param new_domain the new domain as a pair of doubles.
	    void rescaleDomain(std::pair<double, double> new_domain);

	    /// @brief Evaluate the value at x.
	    /// @param x a domain value
	    /// @return f(x)
	    double operator()(const double x) const;

	    /// @brief Evaluate the derivative at x.
	    /// @param x a domain value
	    /// @return f'(x)
	    double derivative(const double x) const;

	    /// @brief Evaluate the inverse at y. Requires T to be a double.
	    /// @param y a range value
	    /// @return f^{-1}(y)
	    double inverse(const double y) const;

	    /// @brief Equality operator.
	    /// @param other another NonuniformTableLinear.
	    /// @return true if they are represented exactly alike.
	    bool operator==(const NonuniformTableLinear& other) const;

	    /// @brief Policies for how to behave when trying to evaluate outside the domain.
	    enum RangePolicy {Throw = 0, ClosestValue = 1, Extrapolate = 2};

	    /// @brief Sets the behavioural policy for evaluation to the left of the domain.
	    /// @param rp the policy
	    void setLeftPolicy(RangePolicy rp);

	    /// @brief Sets the behavioural policy for evaluation to the right of the domain.
	    /// @param rp the policy
	    void setRightPolicy(RangePolicy rp);

	protected:
	    std::vector<double> x_values_;
	    std::vector<T> y_values_;
	    mutable std::vector<T> x_values_reversed_;
	    mutable std::vector<T> y_values_reversed_;
	    RangePolicy left_;
	    RangePolicy right_;
	};


	// A utility function
	/// @brief Detect if a sequence is nondecreasing.
	/// @tparam FI a forward iterator whose value type has operator< defined.
	/// @param beg start of sequence
	/// @param end one-beyond-end of sequence
	/// @return false if there exists two consecutive values (v1, v2) in the sequence
	///         for which v2 < v1, else returns true.
	template <typename FI>
	bool isNondecreasing(const FI beg, const FI end)
	{
	    if (beg == end) return true;
	    FI it = beg;
	    ++it;
	    FI prev = beg;
	    for (; it != end; ++it, ++prev) {
		if (*it < *prev) {
		    return false;
		}
	    }
	    return true;
	}



	// Member implementations.

	template<typename T>
	inline
	NonuniformTableLinear<T>
	::NonuniformTableLinear()
	    : left_(ClosestValue), right_(ClosestValue)
	{
	}

	template<typename T>
	inline
	NonuniformTableLinear<T>
	::NonuniformTableLinear(const std::vector<double>& x_values,
				const std::vector<T>& y_values)
	    : x_values_(x_values), y_values_(y_values),
	      left_(ClosestValue), right_(ClosestValue)
	{
	    ASSERT(isNondecreasing(x_values.begin(), x_values.end()));
	}

	template<typename T>
	inline std::pair<double, double>
	NonuniformTableLinear<T>
	::domain()
	{
	    return std::make_pair(x_values_[0], x_values_.back());
	}

	template<typename T>
	inline void
	NonuniformTableLinear<T>
	::rescaleDomain(std::pair<double, double> new_domain)
	{
	    const double a = x_values_[0];
	    const double b = x_values_.back();
	    const double c = new_domain.first;
	    const double d = new_domain.second;
	    // x in [a, b] -> x in [c, d]
	    for (int i = 0; i < int(x_values_.size()); ++i) {
		x_values_[i] = (x_values_[i] - a)*(d - c)/(b - a) + c;
	    }
	}

	template<typename T>
	inline double
	NonuniformTableLinear<T>
	::operator()(const double x) const
	{
	    return linearInterpolation(x_values_, y_values_, x);
	}

	template<typename T>
	inline double
	NonuniformTableLinear<T>
	::derivative(const double x) const
	{
	    return linearInterpolationDerivative(x_values_, y_values_, x);
	}

	template<typename T>
	inline double
	NonuniformTableLinear<T>
	::inverse(const double y) const
	{
            if (y_values_.front() < y_values_.back()) {
                return linearInterpolation(y_values_, x_values_, y);
            } else {
                if (y_values_reversed_.empty()) {
                    y_values_reversed_ = y_values_;
                    std::reverse(y_values_reversed_.begin(), y_values_reversed_.end());
                    ASSERT(isNondecreasing(y_values_reversed_.begin(), y_values_reversed_.end()));
                    x_values_reversed_ = x_values_;
                    std::reverse(x_values_reversed_.begin(), x_values_reversed_.end());
                }
                return linearInterpolation(y_values_reversed_, x_values_reversed_, y);
            }
	}

	template<typename T>
	inline bool
	NonuniformTableLinear<T>
	::operator==(const NonuniformTableLinear<T>& other) const
	{
	    return x_values_ == other.x_values_
		&& y_values_ == other.y_values_
		&& left_ == other.left_
		&& right_ == other.right_;
	}

	template<typename T>
	inline void
	NonuniformTableLinear<T>
	::setLeftPolicy(RangePolicy rp)
	{
	    if (rp != ClosestValue) {
		THROW("Only ClosestValue RangePolicy implemented.");
	    }
	    left_ = rp;
	}

	template<typename T>
	inline void
	NonuniformTableLinear<T>
	::setRightPolicy(RangePolicy rp)
	{
	    if (rp != ClosestValue) {
		THROW("Only ClosestValue RangePolicy implemented.");
	    }
	    right_ = rp;
	}

    } // namespace utils
} // namespace Dune

#endif // OPENRS_NONUNIFORMTABLELINEAR_HEADER
