/* 
 * Copyright (C) 2006
 * Swiss Federal Institute of Technology, Zurich. All rights reserved.
 * 
 * Developed at the Autonomous Systems Lab.
 * Visit our homepage at http://www.asl.ethz.ch/
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


#ifndef NPM_BICYCLEDRIVEDRAWING_HPP
#define NPM_BICYCLEDRIVEDRAWING_HPP


#include <npm/common/Drawing.hpp>
#include <npm/common/BicycleDrive.hpp>

namespace npm {

  
  class BicycleDriveDrawing
    : public Drawing
  {
  public:
    BicycleDriveDrawing(const std::string & name,
		     boost::shared_ptr<const BicycleDrive> drive);
    
    virtual void Draw();
    
  private:
    boost::shared_ptr<const BicycleDrive> m_drive;
    const double m_wheelbase;
    const double m_wheelradius;
    const double m_axlewidth;

  };

}

#endif // NPM_BICYCLEDRIVEDRAWING_HPP
