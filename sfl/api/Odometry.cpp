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


#include "Odometry.hpp"
#include "HAL.hpp"
#include "Pose.hpp"
#include <iostream>


using namespace boost;
using namespace std;


namespace sfl {
  
  
  Odometry::
  Odometry(shared_ptr<HAL> hal)
    : m_hal(hal)
  {
  }
  
  
  int Odometry::
  Init(const Pose & pose, ostream * dbgos)
  {
    // don't set timestamp here, will be read from HAL afterwards
    int res(m_hal->odometry_set(pose.X(), pose.Y(), pose.Theta(),
				pose.Sxx(), pose.Syy(), pose.Stt(),
				pose.Sxy(), pose.Sxt(), pose.Syt()));
    if(res != 0){
      if(dbgos != 0)
	(*dbgos) << "ERROR in Odometry::Init():\n"
		 << "  odometry_set() returned " << res << "\n";
      return res;
    }
    timespec_t timestamp;
    res = m_hal->time_get(&timestamp);
    if(res != 0){
      if(dbgos != 0)
	(*dbgos) << "ERROR in Odometry::Init():\n"
		 << "  time_get() returned " << res << "\n";
      return res;
    }
    m_history.clear();
    m_history.insert(make_pair(timestamp, shared_ptr<Pose>(new Pose(pose))));
    return 0;
  }
  
  
  int Odometry::
  Update(ostream * dbgos)
  {
    timespec_t timestamp;
    double x, y, t, sxx, syy, stt, sxy, sxt, syt;
    int res(m_hal->odometry_get(&timestamp,
				&x, &y, &t,
				&sxx, &syy, &stt,
				&sxy, &sxt, &syt));
    if(res != 0){
      if(dbgos != 0)
	(*dbgos) << "ERROR in Odometry::Update():\n"
		 << "  odometry_get() returned " << res << "\n";
      return res;
    }
    shared_ptr<Pose> pose(new Pose(x, y, t, sxx, syy, stt, sxy, sxt, syt));
    m_history.insert(make_pair(timestamp, pose));    
    return 0;
  }
  
  
  shared_ptr<const Pose> Odometry::
  Get() const
  {
    shared_ptr<Pose> pose;
    if(m_history.empty())
      pose.reset(new Pose());
    else
      pose = m_history.rbegin()->second;
    return pose;
  }
  
 
  Odometry::history_t::value_type Odometry::
  Get(const Timestamp & t) const
  {
    if(m_history.empty())
      return make_pair(Timestamp::first, shared_ptr<Pose>());
    
    history_t::const_iterator cand1(m_history.lower_bound(t));
    if(cand1 == m_history.end())
      return *m_history.rbegin();
    if((cand1->first == t) || (cand1 == m_history.begin()))
      return *cand1;
    
    history_t::const_iterator cand2(cand1--);
    Timestamp t1(t);
    t1 -= cand1->first;
    Timestamp t2(cand2->first);
    t2 -= t;
    if(t1 < t2)
      return *cand1;
    return *cand2;
  }
  
  
  int Odometry::
  Set(const Pose & pose)
  {
    // don't set timestamp here, use HAL's time function afterwards
    int res(m_hal->odometry_set(pose.X(), pose.Y(), pose.Theta(),
				pose.Sxx(), pose.Syy(), pose.Stt(),
				pose.Sxy(), pose.Sxt(), pose.Syt()));
    if(res != 0)
      return res;
    timespec_t timestamp;
    res = m_hal->time_get(&timestamp);
    if(res != 0)
      return res;
    m_history.insert(make_pair(timestamp, shared_ptr<Pose>(new Pose(pose))));
    return 0;
  }
  
}
