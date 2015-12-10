#ifndef OPTIMET_TYPES_H
#define OPTIMET_TYPES_H

#include <complex>
#include <functional>
#include <Eigen/Core>

namespace optimet {

  //! Root of the type hierarchy for signed integers
  typedef int t_int;
  //! Root of the type hierarchy for unsigned integers
  typedef size_t t_uint;
  //! Root of the type hierarchy for real numbers
  typedef double t_real;
  //! Root of the type hierarchy for (real) complex numbers
  typedef std::complex<t_real> t_complex;


  //! \brief A vector of a given type
  //! \details Operates as mathematical vector.
  template<class T = t_complex>
    using Vector = Eigen::Matrix<T, Eigen::Dynamic, 1>;
  //! \brief A matrix of a given type
  //! \details Operates as mathematical matrix.
  template<class T = t_complex>
    using Matrix = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;
}
#endif
