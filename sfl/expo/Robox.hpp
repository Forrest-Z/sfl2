/* 
 * Copyright (C) 2004
 * Swiss Federal Institute of Technology, Lausanne. All rights reserved.
 * 
 * Developed at the Autonomous Systems Lab.
 * Visit our homepage at http://asl.epfl.ch/
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifndef EXPO_ROBOX_HPP
#define EXPO_ROBOX_HPP

#include <boost/shared_ptr.hpp>
#include <stdexcept>

namespace sfl {
  class DynamicWindow;
  class RobotModel;
  class Hull;
  class BubbleBand;
  class Odometry;
  class Scanner;
  class Multiscanner;
  class HAL;
  class Goal;
  class Pose;
}

struct expo_parameters;

namespace expo {

  class MotionPlanner;
  class MotionController;
  
  
  class Robox
  {
  public:
    Robox(expo_parameters const & params,
	  boost::shared_ptr<sfl::HAL> hal,
	  boost::shared_ptr<sfl::Multiscanner> mscan);
    
    void SetGoal(double timestep, sfl::Goal const & goal);
    
    sfl::Goal const & GetGoal() const;
    
    /** Calls UpdateMultiscanner(), UpdateMotionPlanner(),
	UpdateMotionController(), and UpdateOdometry(). It throws an
	exception if anything goes wrong. */
    void Update(double timestep) throw(std::runtime_error);
    
    /** Initialize or set the odometry, depending on whether it
	already has been initialized. */
    int SetOdometry(sfl::Pose const & pose, std::ostream * err_os);
    
    /** Get the current pose. If odometry has not been initialized,
	it'll return a default constructed pose (0, 0, 0, 1, 1, 1, 0,
	0, 0). */
    boost::shared_ptr<sfl::Pose const> GetOdometry() const;
    
    int UpdateOdometry(std::ostream * err_os);
    
    bool UpdateMultiscanner(std::ostream * erros);
    
    bool UpdateMotionPlanner(double timestep);
    
    int UpdateMotionController(double timestep, std::ostream * err_os);
    
    static boost::shared_ptr<sfl::Hull> CreateHull();    
    
    
    boost::shared_ptr<sfl::Hull> hull;
    boost::shared_ptr<sfl::RobotModel> robotModel;
    boost::shared_ptr<MotionController> motionController;
    boost::shared_ptr<sfl::DynamicWindow> dynamicWindow;
    boost::shared_ptr<sfl::Odometry> odometry;
    boost::shared_ptr<sfl::BubbleBand> bubbleBand;
    boost::shared_ptr<sfl::Multiscanner> mscan;
    boost::shared_ptr<MotionPlanner> motionPlanner;
  };

}

#endif // EXPO_ROBOX_HPP