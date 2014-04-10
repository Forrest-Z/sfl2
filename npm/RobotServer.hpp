/* 
 * Copyright (C) 2006 Roland Philippsen <roland dot philippsen at gmx dot net>
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


#ifndef NPM_ROBOTSERVER_HPP
#define NPM_ROBOTSERVER_HPP


#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>


namespace sfl {
  class Line;
  class Frame;
  class Scanner;
}


namespace npm {

  class RobotClient;  
  class Sensor;
  class Object;
  class World;
  class Lidar;
  class Sharp;
  class HAL;
  class Drawing;
  class Camera;
  class Drive;
  class DiffDrive;
  class HoloDrive;
  class BicycleDrive;
  class NoiseModel;
  class KeyListener;
  class CheatSheet;
  
  
  class RobotServer
  {
  public:
    typedef std::vector<boost::shared_ptr<const sfl::Frame> > trajectory_t;
    
    RobotServer(RobotClient *client,
		World const &world);
    
    void UpdateAllSensors();
    void UpdateSensor(Sensor & sensor) const;
    void SimulateAction(double timestep);
    void InitializePose(const sfl::Frame & pose);
    void AddPose(boost::shared_ptr<const sfl::Frame> pose);
    
  private:
    void AddTruePose(boost::shared_ptr<const sfl::Frame> pose);
    void AddNoisyPose(boost::shared_ptr<const sfl::Frame> pose);
    
  public:
    const std::string & GetName() const;
    const sfl::Frame & GetTruePose() const;
    const sfl::Frame * GetNoisyPose() const;
    const Object & GetBody() const;
    const Object * GetNoisyBody() const;
    const trajectory_t & GetTrueTrajectory() const;
    const trajectory_t * GetNoisyTrajectory() const;
    const World & GetWorld() const;
    boost::shared_ptr<const Lidar> GetLidar(int channel) const;
    boost::shared_ptr<const Sharp> GetSharp(int channel) const;
    boost::shared_ptr<HAL> GetHAL();
    
    void AddLine(double x0, double y0, double x1, double y1);
    void AddLine(const sfl::Line & line);

  private:
    void AddDrawing(Drawing * drawing);
    void AddCamera(Camera * camera);
    
  public:
    
    boost::shared_ptr<Lidar>
    DefineLidar(const sfl::Frame & mount, size_t nscans, double rhomax,
		double phi0, double phirange, int hal_channel);

    boost::shared_ptr<Lidar>
    DefineLidar(boost::shared_ptr<sfl::Scanner> scanner);

    boost::shared_ptr<Sharp>
    DefineSharp(const sfl::Frame & mount, double rmax, int channel);
    
    boost::shared_ptr<DiffDrive>
    DefineDiffDrive(double wheelbase, double wheelradius);
    
    boost::shared_ptr<HoloDrive>
    DefineHoloDrive(double axislength);

    boost::shared_ptr<BicycleDrive>
    DefineBicycleDrive(double wheelbase, double wheelradius, double axlewidth);
    
    void AddKeyListener(boost::shared_ptr<KeyListener> listener) const;
    boost::shared_ptr<npm::CheatSheet> CreateCheatSheet() const;
    
    RobotClient const * GetClient() const { return m_client; }
    
  private:
    friend class Simulator;
    
    RobotClient *m_client;
    World const &m_world;
    boost::shared_ptr<HAL> m_hal;
    std::vector<boost::shared_ptr<Drawing> > m_drawing;
    std::vector<boost::shared_ptr<Camera> > m_camera;
    std::vector<boost::shared_ptr<Sensor> > m_sensor;
    boost::shared_ptr<Drive> m_drive;
    std::map<int, boost::shared_ptr<Lidar> > m_lidar;
    std::map<int, boost::shared_ptr<Sharp> > m_sharp;
    boost::shared_ptr<Object> m_true_body;
    boost::shared_ptr<Object> m_noisy_body;
    trajectory_t m_true_trajectory;
    boost::shared_ptr<trajectory_t> m_noisy_trajectory;
    boost::shared_ptr<sfl::Frame> m_true_pose;
    boost::shared_ptr<sfl::Frame> m_noisy_pose;
    boost::shared_ptr<NoiseModel> m_scanner_noise_model;
    boost::shared_ptr<NoiseModel> m_odometry_noise_model;
  };
  
}

#endif // NPM_ROBOTSERVER_HPP
