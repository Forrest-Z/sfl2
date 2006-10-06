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


#include "Drawing.hpp"
#include "Manager.hpp"
#include <boost/shared_ptr.hpp>


using namespace boost;
using namespace std;


namespace npm {

  
  template<>
  shared_ptr<UniqueManager<Drawing> > Instance()
  {
    static shared_ptr<UniqueManager<Drawing> > instance;
    if( ! instance)
      instance.reset(new UniqueManager<Drawing>());
    return instance;
  }
  
  
  Drawing::
  Drawing(const string & name):
    Manageable(name)
  {
    Instance<UniqueManager<Drawing> >()->Attach(this);
  }
  
  
  Drawing::
  ~Drawing()
  {
    Instance<UniqueManager<Drawing> >()->Detach(this);
  }
  
}
