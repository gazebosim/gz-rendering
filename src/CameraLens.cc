/*
 * Copyright (C) 2021 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <ignition/common/Console.hh>
#include <ignition/math/Vector3.hh>

#include "ignition/rendering/CameraLens.hh"

/// \brief Private fields of camera lens
class ignition::rendering::CameraLensPrivate
{
  /// \brief Linear scale factor
  public: double c1 = 1.0;

  /// \brief Angle scale factor
  public: double c2 = 1.0;

  /// \brief Angle offset factor
  public: double c3 = 0.0;

  /// \brief Linear scale factor, may be adjusted in runtime
  public: double f = 1.0;

  /// \brief Visible field of view
  public: double cutOffAngle = IGN_PI*0.5;

  /// \brief True to scale image to hfov, false to render as output according
  /// to projection parameters
  public: bool scaleToHFov = true;

  /// \brief Mapping function type
  public: std::string type = "gnomonical";

  /// \brief Enumeration of functions that can be casted to some other types
  public: class MapFunctionEnum
          {
            /// \brief Constructor
            /// \param[in] str Function name 'sin', 'tan' or 'id'
            public: explicit MapFunctionEnum(const std::string &_str)
            {
              variants.push_back(std::make_tuple("sin",
                    math::Vector3d::UnitX,
                    std::function<float (float)>(
                      static_cast<float (*)(float)>(&std::sin))));

              variants.push_back(std::make_tuple("tan",
                    math::Vector3d::UnitY,
                    std::function<float (float)>(
                      static_cast<float (*)(float)>(&std::tan))));

              variants.push_back(std::make_tuple("id",
                    math::Vector3d::UnitZ,
                    std::function<float (float)>(
                      [](float t) -> float
                      {
                      return t;
                      })));

              for (auto item : variants)
              {
                if (std::get<0>(item) == _str)
                {
                  value = item;
                  return;
                }
              }

              // function provided is not in array
              throw std::invalid_argument("Unknown function ["+_str+"]");
            }

            /// \brief Cast to ignition::math::Vector3d,
            ///   this vector is passed to shader to avoid branching
            /// \return Vector3 Vector whose one component is 1
            ///   and the rest are nulls
            public: math::Vector3d AsVector3d() const
            {
              return std::get<1>(value);
            }

            /// \brief Cast to std::string
            /// \return The same string which was passed to constructor
            public: std::string AsString() const
            {
              return std::get<0>(value);
            }

            /// \brief Apply function to float value
            /// \return The result of application
            public: float Apply(float _t)
            {
              return std::get<2>(value)(_t);
            }

            /// \brief Assignment operator
            /// \param[in] _fun Rvalue
            /// \return Reference to (*this)
            public: MapFunctionEnum &operator=(const MapFunctionEnum &_fun)
            {
              this->value = _fun.value;
              return *this;
            }

            /// \brief List of all available functions
            ///   and its associated representations
            private: std::vector<std::tuple<std::string,
                     math::Vector3d,
                       std::function<float (float)> > > variants;

            /// \brief Current value of enumeration
            private: decltype(variants)::value_type value;
          };

  /// \brief `fun` component of the mapping function,
  /// \see CameraLens description
  public: MapFunctionEnum fun = MapFunctionEnum("id");

  /// \brief SDF element of the lens
//  public: sdf::ElementPtr sdf;

  /// \brief Mutex to lock when getting or setting lens data
//  public: std::recursive_mutex dataMutex;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
CameraLens::CameraLens()
  : dataPtr(std::make_unique<CameraLensPrivate>())
{
}

//////////////////////////////////////////////////
CameraLens::CameraLens(const CameraLens &_other)
  : dataPtr(std::make_unique<CameraLensPrivate>())
{
  // Avoid incorrect cppcheck error about dataPtr being assigned in constructor
  CameraLensPrivate &dp = *(this->dataPtr);
  dp = *(_other.dataPtr);
}

//////////////////////////////////////////////////
CameraLens::~CameraLens()
{
}

//////////////////////////////////////////////////
CameraLens &CameraLens::operator=(const CameraLens &_other)
{
  *(this->dataPtr) = *(_other.dataPtr);
  return *this;
}

//////////////////////////////////////////////////
void CameraLens::SetLensFunction(double _c1, double _c2,
    const std::string &_fun, double _f, double _c3)
{
  this->dataPtr->c1 = _c1;
  this->dataPtr->c2 = _c2;
  this->dataPtr->c3 = _c3;
  this->dataPtr->f = _f;

  try
  {
    this->dataPtr->fun = CameraLensPrivate::MapFunctionEnum(_fun);
  }
  catch(const std::exception &ex)
  {
    ignerr << "`fun` value [" << _fun << "] is not known, "
          << "[tan] will be used instead" << std::endl;

    this->dataPtr->fun = CameraLensPrivate::MapFunctionEnum("tan");
  }
}

// //////////////////////////////////////////////////
// void CameraLens::Load()
// {
//   if (!this->dataPtr->sdf->HasElement("type"))
//   {
//     gzwarn << "You should specify lens type using <type> element";
//     this->dataPtr->sdf->AddElement("type");
//   }
//
//   if (this->IsCustom())
//   {
//     if (this->dataPtr->sdf->HasElement("custom_function"))
//     {
//       sdf::ElementPtr cf = this->dataPtr->sdf->GetElement("custom_function");
//
//       this->Init(
//         cf->Get<double>("c1"),
//         cf->Get<double>("c2"),
//         cf->Get<std::string>("fun"),
//         cf->Get<double>("f"),
//         cf->Get<double>("c3"));
//     }
//     else
//     {
//       ignerr << "You need a <custom_function> element to use this lens type, "
//             << "setting lens type to `stereographic`" << std::endl;
//
//       this->dataPtr->sdf->GetElement("type")->Set("stereographic");
//       this->SetLensFunction(this->Type());
//     }
//   }
//   else
//     this->SetLensFunction(this->Type());
//
//   this->SetCutOffAngle(this->dataPtr->sdf->Get<double>("cutoff_angle"));
// }

//////////////////////////////////////////////////
std::string CameraLens::Type() const
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);
  return this->dataPtr->type;
}

//////////////////////////////////////////////////
bool CameraLens::IsCustom() const
{
  return this->Type() == "custom";
}

//////////////////////////////////////////////////
double CameraLens::C1() const
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

  return this->dataPtr->c1;
}

//////////////////////////////////////////////////
double CameraLens::C2() const
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

  return this->dataPtr->c2;
}

//////////////////////////////////////////////////
double CameraLens::C3() const
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

  return this->dataPtr->c3;
}

//////////////////////////////////////////////////
double CameraLens::F() const
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

  return this->dataPtr->f;
}

//////////////////////////////////////////////////
std::string CameraLens::Fun() const
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

  return this->dataPtr->fun.AsString();
}

//////////////////////////////////////////////////
double CameraLens::CutOffAngle() const
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

  return this->dataPtr->cutOffAngle;
}

//////////////////////////////////////////////////
bool CameraLens::ScaleToHFOV() const
{
  return this->dataPtr->scaleToHFov;
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

//   return this->dataPtr->sdf->Get<bool>("scale_to_hfov");
}

//////////////////////////////////////////////////
void CameraLens::SetType(const std::string &_type)
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

  // c1, c2, c3, f, fun
  std::map< std::string, std::tuple<double, double, double,
      double, std::string> > funTypes = {
    {"gnomonical",      std::make_tuple(1.0, 1.0, 0.0, 1.0, "tan")},
    {"stereographic",   std::make_tuple(2.0, 2.0, 0.0, 1.0, "tan")},
    {"equidistant",     std::make_tuple(1.0, 1.0, 0.0, 1.0, "id")},
    {"equisolid_angle", std::make_tuple(2.0, 2.0, 0.0, 1.0, "sin")},
    {"orthographic",    std::make_tuple(1.0, 1.0, 0.0, 1.0, "sin")}};

  funTypes.emplace("custom",
      std::make_tuple(this->C1(), this->C2(), this->C3(), this->F(),
        CameraLensPrivate::MapFunctionEnum(this->Fun()).AsString()));

  decltype(funTypes)::mapped_type params;

  try
  {
    params = funTypes.at(_type);
  }
  catch(...)
  {
    ignerr << "Unknown lens type [" << _type << "]" << std::endl;
    return;
  }

//  this->dataPtr->sdf->GetElement("type")->Set(_type);
  this->dataPtr->type = _type;

  if (_type == "custom")
  {
    this->SetC1(std::get<0>(params));
    this->SetC2(std::get<1>(params));
    this->SetC3(std::get<2>(params));
    this->SetF(std::get<3>(params));
    this->SetFun(std::get<4>(params));
  }
  else
  {
    // Do not write values to SDF
    this->dataPtr->c1 = std::get<0>(params);
    this->dataPtr->c2 = std::get<1>(params);
    this->dataPtr->c3 = std::get<2>(params);
    this->dataPtr->f = std::get<3>(params);

    try
    {
      this->dataPtr->fun =
          CameraLensPrivate::MapFunctionEnum(std::get<4>(params));
    }
    catch(const std::exception &ex)
    {
      ignerr << "`fun` value [" << std::get<4>(params)
            << "] is not known, keeping the old one" << std::endl;
    }
  }
}

//////////////////////////////////////////////////
void CameraLens::SetC1(double _c)
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

  this->dataPtr->c1 = _c;

  if (!this->IsCustom())
    this->ConvertToCustom();

//  this->dataPtr->sdf->GetElement("custom_function")->GetElement("c1")->Set(_c);
}

//////////////////////////////////////////////////
void CameraLens::SetC2(double _c)
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

  this->dataPtr->c2 = _c;

  if (!this->IsCustom())
    this->ConvertToCustom();

//  this->dataPtr->sdf->GetElement("custom_function")->GetElement("c2")->Set(_c);
}

//////////////////////////////////////////////////
void CameraLens::SetC3(double _c)
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

  this->dataPtr->c3 = _c;

  if (!this->IsCustom())
    this->ConvertToCustom();

//  this->dataPtr->sdf->GetElement("custom_function")->GetElement("c3")->Set(_c);
}

//////////////////////////////////////////////////
void CameraLens::SetF(double _f)
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

  this->dataPtr->f = _f;

  if (!this->IsCustom())
    this->ConvertToCustom();

//  this->dataPtr->sdf->GetElement("custom_function")->GetElement("f")->Set(_f);
}

//////////////////////////////////////////////////
void CameraLens::SetFun(const std::string &_fun)
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

  if (!this->IsCustom())
    this->ConvertToCustom();

  try
  {
    this->dataPtr->fun = CameraLensPrivate::MapFunctionEnum(_fun);
  }
  catch(const std::exception &ex)
  {
    ignerr << "`Fun` value [" << _fun << "] is not known, "
          << "keeping the old one" << std::endl;
    return;
  }

//  auto customFunction = this->dataPtr->sdf->GetElement("custom_function");
//  customFunction->GetElement("fun")->Set(_fun);
}

//////////////////////////////////////////////////
void CameraLens::SetCutOffAngle(const double _angle)
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

  this->dataPtr->cutOffAngle = _angle;

//  this->dataPtr->sdf->GetElement("cutoff_angle")->Set(_angle);
}

//////////////////////////////////////////////////
void CameraLens::SetScaleToHFOV(bool _scale)
{
  this->dataPtr->scaleToHFov = _scale;
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

//  this->dataPtr->sdf->GetElement("scale_to_hfov")->Set(_scale);
}

//////////////////////////////////////////////////
// void CameraLens::SetUniformVariables(::Pass *_pass, const float _ratio,
//                                      const float _hfov)
// {
//   ::GpuProgramParametersSharedPtr uniforms =
//     _pass->getFragmentProgramParameters();
//
//   uniforms->setNamedConstant("c1", static_cast<::Real>(this->dataPtr->c1));
//   uniforms->setNamedConstant("c2", static_cast<::Real>(this->dataPtr->c2));
//   uniforms->setNamedConstant("c3", static_cast<::Real>(this->dataPtr->c3));
//
//   if (this->ScaleToHFOV())
//   {
//     float param = (_hfov/2)/this->dataPtr->c2+this->dataPtr->c3;
//     float funRes = this->dataPtr->fun.Apply(static_cast<float>(param));
//
//     float newF = 1.0f/(this->dataPtr->c1*funRes);
//
//     uniforms->setNamedConstant("f", static_cast<::Real>(newF));
//   }
//   else
//     uniforms->setNamedConstant("f", static_cast<::Real>(this->dataPtr->f));
//
//   auto vecFun = this->dataPtr->fun.AsVector3d();
//
//   uniforms->setNamedConstant("fun", ::Vector3(
//       vecFun.X(), vecFun.Y(), vecFun.Z()));
//
//   uniforms->setNamedConstant("cutOffAngle",
//     static_cast<::Real>(this->dataPtr->cutOffAngle));
//
//   ::GpuProgramParametersSharedPtr uniforms_vs =
//     _pass->getVertexProgramParameters();
//
//   uniforms_vs->setNamedConstant("ratio", static_cast<::Real>(_ratio));
// }

//////////////////////////////////////////////////
void CameraLens::ConvertToCustom()
{
//  std::lock_guard<std::recursive_mutex> lock(this->dataPtr->dataMutex);

  this->SetType("custom");

//   sdf::ElementPtr cf = this->dataPtr->sdf->AddElement("custom_function");
//
//   cf->AddElement("c1")->Set(this->dataPtr->c1);
//   cf->AddElement("c2")->Set(this->dataPtr->c2);
//   cf->AddElement("c3")->Set(this->dataPtr->c3);
//   cf->AddElement("f")->Set(this->dataPtr->f);
//
//   cf->AddElement("fun")->Set(this->dataPtr->fun.AsString());
}

//////////////////////////////////////////////////
float CameraLens::ApplyMappingFunction(float _f) const
{
  return this->dataPtr->fun.Apply(_f);
}

//////////////////////////////////////////////////
math::Vector3d CameraLens::MappingFunctionAsVector3d() const
{
  return this->dataPtr->fun.AsVector3d();
}
