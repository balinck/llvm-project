//===--- ROCm.h - ROCm installation detector --------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ROCM_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ROCM_H

#include "clang/Basic/Cuda.h"
#include "clang/Basic/LLVM.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Options.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/VersionTuple.h"

namespace clang {
namespace driver {

/// A class to find a viable ROCM installation
/// TODO: Generalize to handle libclc.
class RocmInstallationDetector {
private:
  struct ConditionalLibrary {
    SmallString<0> On;
    SmallString<0> Off;

    bool isValid() const { return !On.empty() && !Off.empty(); }

    StringRef get(bool Enabled) const {
      assert(isValid());
      return Enabled ? On : Off;
    }
  };

  // Installation path candidate.
  struct Candidate {
    llvm::SmallString<0> Path;
    bool StrictChecking;
    // Release string for ROCm packages built with SPACK if not empty. The
    // installation directories of ROCm packages built with SPACK follow the
    // convention <package_name>-<rocm_release_string>-<hash>.
    std::string SPACKReleaseStr;

    bool isSPACK() const { return !SPACKReleaseStr.empty(); }
    Candidate(std::string Path, bool StrictChecking = false,
              StringRef SPACKReleaseStr = {})
        : Path(Path), StrictChecking(StrictChecking),
          SPACKReleaseStr(SPACKReleaseStr.str()) {}
  };

  const Driver &D;
  bool HasHIPRuntime = false;
  bool HasDeviceLibrary = false;

  // Default version if not detected or specified.
  const unsigned DefaultVersionMajor = 3;
  const unsigned DefaultVersionMinor = 5;
  const char *DefaultVersionPatch = "0";

  // The version string in Major.Minor.Patch format.
  std::string DetectedVersion;
  // Version containing major and minor.
  llvm::VersionTuple VersionMajorMinor;
  // Version containing patch.
  std::string VersionPatch;

  // ROCm path specified by --rocm-path.
  StringRef RocmPathArg;
  // ROCm device library paths specified by --rocm-device-lib-path.
  std::vector<std::string> RocmDeviceLibPathArg;
  // HIP runtime path specified by --hip-path.
  StringRef HIPPathArg;
  // HIP version specified by --hip-version.
  StringRef HIPVersionArg;
  // Wheter -nogpulib is specified.
  bool NoBuiltinLibs = false;

  // Paths
  SmallString<0> InstallPath;
  SmallString<0> BinPath;
  SmallString<0> LibPath;
  SmallString<0> LibDevicePath;
  SmallString<0> IncludePath;
  llvm::StringMap<std::string> LibDeviceMap;

  // Libraries that are always linked.
  SmallString<0> OCML;
  SmallString<0> OCKL;

  // Libraries that are always linked depending on the language
  SmallString<0> OpenCL;
  SmallString<0> HIP;

  // Asan runtime library
  SmallString<0> AsanRTL;

  // Libraries swapped based on compile flags.
  ConditionalLibrary WavefrontSize64;
  ConditionalLibrary FiniteOnly;
  ConditionalLibrary UnsafeMath;
  ConditionalLibrary DenormalsAreZero;
  ConditionalLibrary CorrectlyRoundedSqrt;

  // Cache ROCm installation search paths.
  SmallVector<Candidate, 4> ROCmSearchDirs;
  bool PrintROCmSearchDirs;
  bool Verbose;

  bool allGenericLibsValid() const {
    return !OCML.empty() && !OCKL.empty() && !OpenCL.empty() && !HIP.empty() &&
           WavefrontSize64.isValid() && FiniteOnly.isValid() &&
           UnsafeMath.isValid() && DenormalsAreZero.isValid() &&
           CorrectlyRoundedSqrt.isValid();
  }

  void scanLibDevicePath(llvm::StringRef Path);
  bool parseHIPVersionFile(llvm::StringRef V);
  const SmallVectorImpl<Candidate> &getInstallationPathCandidates();

  /// Find the path to a SPACK package under the ROCm candidate installation
  /// directory if the candidate is a SPACK ROCm candidate. \returns empty
  /// string if the candidate is not SPACK ROCm candidate or the requested
  /// package is not found.
  llvm::SmallString<0> findSPACKPackage(const Candidate &Cand,
                                        StringRef PackageName);

public:
  RocmInstallationDetector(const Driver &D, const llvm::Triple &HostTriple,
                           const llvm::opt::ArgList &Args,
                           bool DetectHIPRuntime = true,
                           bool DetectDeviceLib = false);

  /// Get file paths of default bitcode libraries common to AMDGPU based
  /// toolchains.
  llvm::SmallVector<std::string, 12> getCommonBitcodeLibs(
      const llvm::opt::ArgList &DriverArgs, StringRef LibDeviceFile,
      bool Wave64, bool DAZ, bool FiniteOnly, bool UnsafeMathOpt,
      bool FastRelaxedMath, bool CorrectSqrt, bool isOpenMP) const;

  /// Check whether we detected a valid HIP runtime.
  bool hasHIPRuntime() const { return HasHIPRuntime; }

  /// Check whether we detected a valid ROCm device library.
  bool hasDeviceLibrary() const { return HasDeviceLibrary; }

  /// Print information about the detected ROCm installation.
  void print(raw_ostream &OS) const;

  /// Get the detected Rocm install's version.
  // RocmVersion version() const { return Version; }

  /// Get the detected Rocm installation path.
  StringRef getInstallPath() const { return InstallPath; }

  /// Get the detected path to Rocm's bin directory.
  // StringRef getBinPath() const { return BinPath; }

  /// Get the detected Rocm Include path.
  StringRef getIncludePath() const { return IncludePath; }

  /// Get the detected Rocm library path.
  StringRef getLibPath() const { return LibPath; }

  /// Get the detected Rocm device library path.
  StringRef getLibDevicePath() const { return LibDevicePath; }

  StringRef getOCMLPath() const {
    assert(!OCML.empty());
    return OCML;
  }

  StringRef getOCKLPath() const {
    assert(!OCKL.empty());
    return OCKL;
  }

  StringRef getOpenCLPath() const {
    assert(!OpenCL.empty());
    return OpenCL;
  }

  StringRef getHIPPath() const {
    assert(!HIP.empty());
    return HIP;
  }

  /// Returns empty string of Asan runtime library is not available.
  StringRef getAsanRTLPath() const { return AsanRTL; }

  StringRef getWavefrontSize64Path(bool Enabled) const {
    return WavefrontSize64.get(Enabled);
  }

  StringRef getFiniteOnlyPath(bool Enabled) const {
    return FiniteOnly.get(Enabled);
  }

  StringRef getUnsafeMathPath(bool Enabled) const {
    return UnsafeMath.get(Enabled);
  }

  StringRef getDenormalsAreZeroPath(bool Enabled) const {
    return DenormalsAreZero.get(Enabled);
  }

  StringRef getCorrectlyRoundedSqrtPath(bool Enabled) const {
    return CorrectlyRoundedSqrt.get(Enabled);
  }

  /// Get libdevice file for given architecture
  std::string getLibDeviceFile(StringRef Gpu) const {
    return LibDeviceMap.lookup(Gpu);
  }

  void AddHIPIncludeArgs(const llvm::opt::ArgList &DriverArgs,
                         llvm::opt::ArgStringList &CC1Args) const;

  void detectDeviceLibrary();
  void detectHIPRuntime();

  /// Get the values for --rocm-device-lib-path arguments
  std::vector<std::string> getRocmDeviceLibPathArg() const {
    return RocmDeviceLibPathArg;
  }

  /// Get the value for --rocm-path argument
  StringRef getRocmPathArg() const { return RocmPathArg; }

  /// Get the value for --hip-version argument
  StringRef getHIPVersionArg() const { return HIPVersionArg; }

  std::string getHIPVersion() const { return DetectedVersion; }
};

} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ROCM_H
