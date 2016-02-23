#include <iostream>
#include "catch.hpp"

#include "Types.h"
#include "Geometry.h"
#include "Solver.h"
#include "Aliases.h"
#include "constants.h"
#include "HarmonicsIterator.h"

using namespace optimet;

TEST_CASE("Regression for getTLocal") {
  auto const nMax = 7;
  Geometry geometry;
  geometry.pushObject({{0, 0, 0}, {0.9e0, 1.1e0}, 0.7, nMax});
  geometry.pushObject({{1.5, 0, 0}, {0.8e0, 0.7e0}, 0.5, nMax});

  auto const wavelength = 14960e-9;
  Spherical<t_real> const vKinc{2 * constant::pi / wavelength, 90 * constant::pi / 180.0,
                                90 * constant::pi / 180.0};
  SphericalP<t_complex> const Eaux{0e0, 1e0, 0e0};
  Excitation excitation{0, Tools::toProjection(vKinc, Eaux), vKinc, nMax};
  excitation.populate();
  geometry.update(&excitation);


  // The old way
  CompoundIterator p;
  std::complex<double> **T = new std::complex<double> *[2 * p.max(nMax)];
  for(p = 0; p < (int)(2 * p.max(nMax)); p++)
    T[p] = new std::complex<double>[2 * p.max(nMax)];

  geometry.getTLocal(excitation.omega, 0, nMax, T);
  Matrix<t_complex> Told(2 * p.max(nMax), 2 * p.max(nMax));

  for(p = 0; p < (int)(2 * p.max(nMax)); p++) {
    for(std::size_t i(0); i < (std::size_t)(2 * p.max(nMax)); ++i)
      Told((int)p, i) = T[(int)p][i];
    delete[] T[p];
  }
  delete[] T;

  // The new way
  auto const Tnew = geometry.getTLocal(excitation.omega, 0, nMax);

  CHECK(Tnew.rows() == Told.rows());
  CHECK(Tnew.cols() == Told.cols());
  CHECK(Tnew.isApprox(Told, 1e-12));
}

//! Makes some protected method available publicly for testing
class OpenedSolver: public Solver {
  public:
    using Solver::Solver;
    using Solver::populateIndirect;
    using Solver::populateIndirectOld;
    using Solver::populateDirect;
    using Solver::populateDirectOld;
    using Solver::convertIndirectOld;
    using Solver::convertIndirect;
    using Solver::solveInternal;
};

TEST_CASE("Regression for populate") {
  auto const nMax = 7;
  Geometry geometry;
  geometry.pushObject({{0, 0, 0}, {0.9e0, 1.1e0}, 0.7, nMax});
  geometry.pushObject({{1.5, 0, 0}, {0.8e0, 0.7e0}, 0.5, nMax});

  auto const wavelength = 14960e-9;
  Spherical<t_real> const vKinc{2 * constant::pi / wavelength, 90 * constant::pi / 180.0,
                                90 * constant::pi / 180.0};
  SphericalP<t_complex> const Eaux{0e0, 1e0, 0e0};
  Excitation excitation{0, Tools::toProjection(vKinc, Eaux), vKinc, nMax};
  excitation.populate();
  geometry.update(&excitation);

  OpenedSolver solver(&geometry, &excitation, O3DSolverIndirect, nMax);

  SECTION("Indirect") {
    solver.populateIndirectOld();
    auto const Sold = solver.S;
    auto const Qold = solver.Q;
    solver.populateIndirect();
    auto const Snew = solver.S;
    auto const Qnew = solver.Q;

    CHECK(Snew.isApprox(Sold, 1e-12));
    CHECK(Qnew.isApprox(Qold, 1e-12));
  }

  SECTION("Direct") {
    solver.populateDirectOld();
    auto const Sold = solver.S;
    auto const Qold = solver.Q;
    solver.populateDirect();
    auto const Snew = solver.S;
    auto const Qnew = solver.Q;

    CHECK(Snew.isApprox(Sold, 1e-12));
    CHECK(Qnew.isApprox(Qold, 1e-12));
  }
}

TEST_CASE("Regression for convertIndirect") {
  auto const nMax = 7;
  auto const flatMax = HarmonicsIterator::max_flat(nMax) - 1;
  Geometry geometry;
  geometry.pushObject({{0, 0, 0}, {0.9e0, 1.1e0}, 0.7, nMax});
  geometry.pushObject({{1.5, 0, 0}, {0.8e0, 0.7e0}, 0.5, nMax});

  auto const wavelength = 14960e-9;
  Spherical<t_real> const vKinc{2 * constant::pi / wavelength, 90 * constant::pi / 180.0,
                                90 * constant::pi / 180.0};
  SphericalP<t_complex> const Eaux{0e0, 1e0, 0e0};
  Excitation excitation{0, Tools::toProjection(vKinc, Eaux), vKinc, nMax};
  excitation.populate();
  geometry.update(&excitation);

  OpenedSolver solver(&geometry, &excitation, O3DSolverIndirect, nMax);

  Vector<t_complex> Xold = Vector<t_complex>::Random(2 * flatMax * geometry.objects.size());
  auto const Xnew = solver.convertIndirect(Xold);
  solver.convertIndirectOld(Xold);

  CHECK(Xnew.isApprox(Xold, 1e-12));
}

TEST_CASE("Regression for solveInternal") {
  auto const nMax = 7;
  auto const flatMax = HarmonicsIterator::max_flat(nMax) - 1;
  Geometry geometry;
  geometry.pushObject({{0, 0, 0}, {0.9e0, 1.1e0}, 0.7, nMax});
  geometry.pushObject({{1.5, 0, 0}, {0.8e0, 0.7e0}, 0.5, nMax});

  auto const wavelength = 14960e-9;
  Spherical<t_real> const vKinc{2 * constant::pi / wavelength, 90 * constant::pi / 180.0,
                                90 * constant::pi / 180.0};
  SphericalP<t_complex> const Eaux{0e0, 1e0, 0e0};
  Excitation excitation{0, Tools::toProjection(vKinc, Eaux), vKinc, nMax};
  excitation.populate();
  geometry.update(&excitation);

  OpenedSolver solver(&geometry, &excitation, O3DSolverIndirect, nMax);

  Vector<t_complex> const input = Vector<t_complex>::Random(2 * flatMax * geometry.objects.size());
  Vector<t_complex> Xold = Vector<t_complex>::Random(2 * flatMax * geometry.objects.size());
  auto const Xnew = solver.solveInternal(input);
  solver.solveInternal(input, Xold);

  CHECK(Xnew.isApprox(Xold, 1e-12));
}