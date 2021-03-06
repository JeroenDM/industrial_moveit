/**
 * @file master_ik.cpp
 * @brief This the master IK solver which loads all constraints from a yaml file.
 *
 * @author Levi Armstrong
 * @date Febuary 15, 2015
 * @version TODO
 * @bug No known bugs
 *
 * @copyright Copyright (c) 2015, Southwest Research Institute
 *
 * @license Software License Agreement (Apache License)\n
 * \n
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at\n
 * \n
 * http://www.apache.org/licenses/LICENSE-2.0\n
 * \n
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MASTER_IK_H
#define MASTER_IK_H

#include "constrained_ik/constrained_ik.h"
#include "constrained_ik/enum_types.h"
#include <pluginlib/class_loader.h>

namespace constrained_ik
{

/** @brief Master IK Solver, This will add contraints defined as plugins in a yaml file. */
class MasterIK : public Constrained_IK
{
public:
  MasterIK(std::string group_name): Constrained_IK()
  {
    constraint_loader_.reset(new pluginlib::ClassLoader<constrained_ik::Constraint>("constrained_ik", "constrained_ik::Constraint"));

    ros::NodeHandle pnh("~");
    XmlRpc::XmlRpcValue constraints_xml;
    std::string constraint_param = "constrained_ik_solver/" + group_name + "/constraints";

    if (pnh.getParam(constraint_param, constraints_xml))
    {
      if(constraints_xml.getType() == XmlRpc::XmlRpcValue::TypeArray)
      {
        addConstraintFromXml(constraints_xml);
      }
      else
      {
        ROS_ERROR("Parameter must be an array: %s", constraint_param.c_str());
      }
    }
    else
    {
      ROS_ERROR("Unable to find ros parameter: %s", constraint_param.c_str());
    }
  }

  ~MasterIK() {}
private:
  void addConstraintFromXml(const XmlRpc::XmlRpcValue& constraints_xml)
  {
    for (int i=0; i<constraints_xml.size(); ++i)
    {
      XmlRpc::XmlRpcValue constraint_xml = constraints_xml[i];

      if (constraint_xml.hasMember("class") &&
                constraint_xml["class"].getType() == XmlRpc::XmlRpcValue::TypeString &&
                constraint_xml.hasMember("primary") &&
                constraint_xml["primary"].getType() == XmlRpc::XmlRpcValue::TypeBoolean)
      {
        std::string class_name = constraint_xml["class"];
        bool is_primary = constraint_xml["primary"];

        Constraint *constraint;
        try
        {
          constraint = constraint_loader_->createUnmanagedInstance(class_name);

          constraint->loadParameters(constraint_xml);
          if (is_primary)
            addConstraint(constraint, constraint_types::Primary);
          else
            addConstraint(constraint, constraint_types::Auxiliary);

        }
        catch (pluginlib::PluginlibException& ex)
        {
          ROS_ERROR("Couldn't load constraint named %s", class_name.c_str());
          ROS_ERROR("Error: %s", ex.what());
          ROS_BREAK();
        }
      }
      else
      {
        ROS_ERROR("Constraint must have class(string) and primary(boolean) members");
      }
    }
  }

protected:
  boost::shared_ptr<pluginlib::ClassLoader<constrained_ik::Constraint> > constraint_loader_;

}; // class MasterIK

} // namespace constrained_ik

#endif // MASTER_IK_H

