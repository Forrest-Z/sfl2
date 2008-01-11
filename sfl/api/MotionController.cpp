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


#include "MotionController.hpp"
#include "Timestamp.hpp"
#include "RobotModel.hpp"
#include "HAL.hpp"
#include <iostream>


using namespace boost;
using namespace std;


namespace sfl {
  
  
  MotionControllerThread::
  MotionControllerThread(const string & name, ostream * _dbgos)
    : SimpleThread(name), dbgos(_dbgos), timestep(-42)
  {
  }
  
  
  void MotionControllerThread::
  Step()
  {
    if(( ! motionController) || (timestep < 0)){
      update_status = -42;
      return;
    }
    update_status = motionController->DoUpdate(timestep, dbgos);
  }
  
  
  MotionController::
  MotionController(shared_ptr<const RobotModel> robotModel,
		   shared_ptr<HAL> hal,
		   shared_ptr<RWlock> rwlock)
    : qdMax(robotModel->QdMax()), qddMax(robotModel->QddMax()),
      sdMax(robotModel->SdMax()), thetadMax(robotModel->ThetadMax()),
      m_robotModel(robotModel), m_hal(hal), m_rwlock(rwlock)
  {
  }
  
  
  int MotionController::
  Update(double timestep, ostream * dbgos)
  {
    if(m_thread){
      m_thread->dbgos = dbgos;
      m_thread->timestep = timestep;
      return m_thread->update_status;
    }
    return DoUpdate(timestep, dbgos);
  }
  
  
  int MotionController::
  DoUpdate(double timestep, ostream * dbgos)
  {
    double cqdl, cqdr;
    int status(m_hal->deprecated_speed_get( & cqdl, & cqdr));
    if(0 != status){
      if(dbgos != 0)
	(*dbgos) << "ERROR in MotionController::Update():\n"
		 << "  HAL::speed_get() returned " << status << "\n";
      return -1;
    }
    if(dbgos != 0){
      m_rwlock->Rdlock();
      (*dbgos) << "INFO from MotionController::Update()\n"
	       << "  timestep: " << timestep << "\n"
	       << "  current:  (" << cqdl << ", " << cqdr << ")\n"
	       << "  proposed: (" << m_proposedQdl << ", "
	       << m_proposedQdr << ")\n";
      m_rwlock->Unlock();
    }
    
    // limit in actuator speed space
    const double dqd(timestep * qddMax);
    const double qdlMax(boundval(- qdMax, cqdl + dqd, qdMax));
    const double qdlMin(boundval(- qdMax, cqdl - dqd, qdMax));
    const double qdrMax(boundval(- qdMax, cqdr + dqd, qdMax));
    const double qdrMin(boundval(- qdMax, cqdr - dqd, qdMax));
    
    m_rwlock->Rdlock();
    const double pqdl(boundval(qdlMin, m_proposedQdl, qdlMax));
    const double pqdr(boundval(qdrMin, m_proposedQdr, qdrMax));
    m_rwlock->Unlock();
    
    // limit in global speed space
    double sd, thetad;
    m_robotModel->Actuator2Global(pqdl, pqdr, sd, thetad);
    sd =     boundval( - sdMax,     sd,     sdMax);
    thetad = boundval( - thetadMax, thetad, thetadMax);
    double wqdl, wqdr;
    m_robotModel->Global2Actuator(sd, thetad, wqdl, wqdr);
    
    // send it
    if(dbgos != 0)
      (*dbgos) << "  wanted:   (" << wqdl << ", " << wqdr << ")\n";
    status = m_hal->deprecated_speed_set(wqdl, wqdr);
    
    // synch cached values before treating status
    m_rwlock->Wrlock();
    m_currentQdl = cqdl;
    m_currentQdr = cqdr;
    m_proposedQdl = pqdl;
    m_proposedQdr = pqdr;
    m_wantedQdl = wqdl;
    m_wantedQdr = wqdr;
    m_rwlock->Unlock();
    
    if(0 != status){
      if(dbgos != 0)
	(*dbgos) << "ERROR in MotionController::Update():\n"
		 << "  HAL::speed_set() returned " << status << "\n";
      return -2;
    }
    return 0;
  }
  
  
  void MotionController::
  ProposeSpeed(double sd, double thetad)
  {
    m_rwlock->Wrlock();
    m_robotModel->Global2Actuator(sd, thetad, m_proposedQdl, m_proposedQdr);
    m_rwlock->Unlock();
  }
  
  
  void MotionController::
  ProposeActuators(double qdLeft, double qdRight)
  {
    m_rwlock->Wrlock();
    m_proposedQdl = qdLeft;
    m_proposedQdr = qdRight;
    m_rwlock->Unlock();
  }
  
  
  void MotionController::
  GetCurrentGlob(double & sd, double & thetad) const
  {
    m_rwlock->Rdlock();
    m_robotModel->Actuator2Global(m_currentQdl, m_currentQdr, sd, thetad);
    m_rwlock->Unlock();
  }
  
  
  void MotionController::
  GetWantedGlob(double & sd, double & thetad) const
  {
    m_rwlock->Rdlock();
    m_robotModel->Actuator2Global(m_wantedQdl, m_wantedQdr, sd, thetad);
    m_rwlock->Unlock();
  }
  
  
  void MotionController::
  GetProposedGlob(double & sd, double & thetad) const
  {
    m_rwlock->Rdlock();
    m_robotModel->Actuator2Global(m_proposedQdl, m_proposedQdr, sd, thetad);
    m_rwlock->Unlock();
  }
  
  
  void MotionController::
  GetCurrentAct(double & qdLeft, double & qdRight) const
  {
    m_rwlock->Rdlock();
    qdLeft = m_currentQdl;
    qdRight = m_currentQdr;
    m_rwlock->Unlock();
  }
  
  
  void MotionController::
  GetWantedAct(double & qdLeft, double & qdRight) const
  {
    m_rwlock->Rdlock();
    qdLeft = m_wantedQdl;
    qdRight = m_wantedQdr;
    m_rwlock->Unlock();
  }
  
  
  void MotionController::
  GetProposedAct(double & qdLeft, double & qdRight) const
  {
    m_rwlock->Rdlock();
    qdLeft = m_proposedQdl;
    qdRight = m_proposedQdr;
    m_rwlock->Unlock();
  }


  bool MotionController::
  SetThread(shared_ptr<MotionControllerThread> thread)
  {
    RWlock::wrsentry sentry(m_rwlock);
    if(m_thread)
      return false;
    m_thread = thread;
    thread->motionController = this;
    return true;
  }
  
}
