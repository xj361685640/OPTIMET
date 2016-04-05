#ifndef OPTIMET_SCALAPACK_BELOS_H_
#define OPTIMET_SCALAPACK_BELOS_H_

#include "Types.h"
#ifdef OPTIMET_BELOS

#include "mpi/Communicator.h"
#include "scalapack/Matrix.h"

#include <BelosLinearProblem.hpp>
#include <BelosOperatorTraits.hpp>
#include <BelosSolverFactory.hpp>
#include <Kokkos_Serial.hpp>
#include <Kokkos_View.hpp>
#include <Teuchos_RCP.hpp>
#include <Tpetra_MultiVector.hpp>

namespace optimet {
namespace scalapack {

//! Simplifies access to a unmanaged kokkos view type
template <class SCALAR, class DEVICE = Kokkos::Serial>
using TeuchosArrayView = Teuchos::ArrayView<SCALAR>;
//! Simplifies access to a tpetra vector type
template <class SCALAR> using TpetraVector = Tpetra::MultiVector<SCALAR>;
//! Simplifies acccess to the Belos operator object
template <class SCALAR> using BelosOperator = std::reference_wrapper<Matrix<SCALAR> const>;
//! Simplifies acccess to the Belos solver factory object
template <class SCALAR>
using BelosSolverFactory =
    Belos::SolverFactory<SCALAR, TpetraVector<SCALAR>, BelosOperator<SCALAR>>;
//! Simplifies acccess to the Belos linear problem object
template <class SCALAR>
using BelosLinearProblem =
    Belos::LinearProblem<SCALAR, TpetraVector<SCALAR>, BelosOperator<SCALAR>>;

//! Obtains a 1-dimensional kokkos view over a matrix data
template <class SCALAR> TeuchosArrayView<SCALAR> view(Matrix<SCALAR> &A);
//! Obtains a 1-dimensional kokkos view over a constant matrix data
template <class SCALAR> TeuchosArrayView<SCALAR const> view(Matrix<SCALAR> const &A);

//! \brief Obtains a map for a scalapack::Matrix
//! \note Belos doesn't know that the matrix is block cyclic. Since we will be defining our own
//! matrix multiply, that's not really necessary.
//! \param[in] A: matrix
//! \param[in] comm: an mpi communicator equivalent to the scalapack context. All processes and only
//!     the processes for which the scalapack context is valid should be in the communicator.
template <class SCALAR>
Teuchos::RCP<const Tpetra::Map<>>
matrix_map(Matrix<SCALAR> const &A, mpi::Communicator const &comm);

//! \brief Obtains a vector for a scalapack matrix
//! \param[in] A: A matrix viewed as a row vector.
//! \param[in] comm: an mpi communicator equivalent to the scalapack context. All processes and only
//!     the processes for which the scalapack context is valid should be in the communicator.
template <class SCALAR>
Teuchos::RCP<TpetraVector<SCALAR>>
tpetra_vector(Matrix<SCALAR> const &A, mpi::Communicator const &comm);

//! Matrix-multiplication against a Belos MultiVector
template <class SCALAR>
void matrix_vector_operator(Matrix<SCALAR> const &matrix, const TpetraVector<SCALAR> &X,
                            TpetraVector<SCALAR> &Y, Belos::ETrans trans = Belos::NOTRANS);

//! \details Creates a scalapack::Matrix view over a Tpetra multi-vector
//! \brief The data in memory is shared. The input vector must have correct (=scalpack) local sizes
//! for the given context, block size, and index.
template <class SCALAR>
Matrix<SCALAR *> as_matrix(TpetraVector<SCALAR> &X, Context const &context, Sizes const &blocks,
                           Index const &index = Index{0, 0});
//! \details Creates a scalapack::Matrix view over a Tpetra multi-vector
//! \details Creates a scalapack::Matrix view over a Tpetra multi-vector
//! \brief Uses the context, block size and index from A.
template <class SCALAR, class OTHER>
Matrix<SCALAR *> as_matrix(TpetraVector<SCALAR> &X, Matrix<OTHER> const &A) {
  return as_matrix(X, A.context(), A.blocks(), A.index());
}
//! \brief The data in memory is shared. The input vector must have correct (=scalpack) local sizes
//! for the given context, block size, and index.
template <class SCALAR>
Matrix<SCALAR const *> as_matrix(const TpetraVector<SCALAR> &X, Context const &context,
                                 Sizes const &blocks, Index const &index = Index{0, 0});
//! \details Creates a scalapack::Matrix view over a Tpetra multi-vector
//! \brief Uses the context, block size and index from A.
template <class SCALAR, class OTHER>
Matrix<SCALAR const *> as_matrix(const TpetraVector<SCALAR> &X, Matrix<OTHER> const &A) {
  return as_matrix(X, A.context(), A.blocks(), A.index());
}
}
}

namespace Belos {
//! Partial specialization of OperatorTraits for Tpetra objects.
template <class SCALAR>
class OperatorTraits<SCALAR, optimet::scalapack::TpetraVector<SCALAR>,
                     optimet::scalapack::BelosOperator<SCALAR>> {
public:
  static void Apply(optimet::scalapack::BelosOperator<SCALAR> const &Op,
                    optimet::scalapack::TpetraVector<SCALAR> const &X,
                    optimet::scalapack::TpetraVector<SCALAR> &Y, const ETrans trans = NOTRANS) {
    optimet::scalapack::matrix_vector_operator(Op.get(), X, Y, trans);
  }

  static bool HasApplyTranspose(optimet::scalapack::Matrix<SCALAR const *> const &) { return true; }
};

// namespace Belos
}
#include "scalapack/Belos.hpp"
#endif
#endif
