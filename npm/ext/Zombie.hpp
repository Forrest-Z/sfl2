/* 
 * Copyright (C) 2004
 * Swiss Federal Institute of Technology, Lausanne. All rights reserved.
 * 
 * Author: Roland Philippsen <roland dot philippsen at gmx dot net>
 *         Autonomous Systems Lab <http://asl.epfl.ch/>
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

#ifndef NPM_EXT_ZOMBIE_HPP
#define NPM_EXT_ZOMBIE_HPP

#include <npm/RobotClient.hpp>

namespace npm {
  
  
  class Zombie:
    public RobotClient
  {
  public:
    Zombie(boost::shared_ptr<RobotDescriptor> descriptor,
	   const World & world);
    
    virtual bool PrepareAction(double timestep);
    virtual void InitPose(double x, double y, double theta);
    virtual void SetPose(double x, double y, double theta);
    virtual void GetPose(double & x, double & y, double & theta);
    virtual void SetGoal(double timestep, const sfl::Goal & goal);
    virtual boost::shared_ptr<const sfl::Goal> GetGoal();
    virtual bool GoalReached();
    
  private:
    boost::shared_ptr<HoloDrive> m_drive;
    boost::shared_ptr<sfl::Goal> m_goal;
  };
  
  
  class LidarZombie:
    public Zombie
  {
  public:
    LidarZombie(boost::shared_ptr<RobotDescriptor> descriptor,
		const World & world);
    
    virtual bool PrepareAction(double timestep);
    
  private:
    boost::shared_ptr<sfl::Scanner> m_scanner;
  };

}

#endif // NPM_EXT_ZOMBIE_HPP