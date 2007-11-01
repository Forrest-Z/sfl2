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


#include "ODrawing.hpp"
#include "../common/wrap_gl.hpp"
#include "../common/Manager.hpp"
#include <sfl/dwa/DynamicWindow.hpp>


using namespace npm;


ODrawing::
ODrawing(const std::string & name,
	 const sfl::Objective & obj,
	 const sfl::DynamicWindow & dwa)
  : Drawing(name,
	    "a DWA's sub-objective (greyscale + special colors)",
	    Instance<UniqueManager<Drawing> >()),
    m_obj(obj),
    m_dwa(dwa)
{
}


void ODrawing::
Draw()
{
  const int qdlMin(m_dwa.QdlMinIndex());
  if(qdlMin < 0)
    return;
  
  const int qdlMax(m_dwa.QdlMaxIndex());
  const int qdrMin(m_dwa.QdrMinIndex());
  const int qdrMax(m_dwa.QdrMaxIndex());
  const double funcMin(m_obj.Min(qdlMin, qdlMax, qdrMin, qdrMax));
  const double funcMax(m_obj.Max(qdlMin, qdlMax, qdrMin, qdrMax));
  double scale;
  if(funcMax - funcMin < sfl::epsilon)
    scale = 0;
  else
    scale = 1 / (funcMax - funcMin);
  
  glPolygonMode(GL_FRONT, GL_FILL);
  double grey;
  for(int l = qdlMin; l <= qdlMax; ++l)
    for(int r = qdrMin; r <= qdrMax; ++r){
      if( ! m_dwa.Forbidden(l, r)){
	grey = scale * (m_obj.Value(l, r) - funcMin);
	glColor3d(grey, grey, grey);
      }
      else
	glColor3d(0.7, 0, 0);

      glRectd(l, r, l + 1, r + 1);

      if(m_dwa.Reachable(l, r)){
	glColor3d(1, 0, 0);
	glBegin(GL_LINES);
	glVertex2d(l,   r);
	glVertex2d(l+1, r+1);
	glVertex2d(l,   r+1);
	glVertex2d(l+1, r);
	glEnd();
      }
    }

  if(m_dwa.QdlOptIndex() >= 0){
    const int qdlOpt(m_dwa.QdlOptIndex());
    const int qdrOpt(m_dwa.QdrOptIndex());
    glPolygonMode(GL_FRONT, GL_LINE);
    glColor3d(0, 1, 1);
    glRectd(qdlOpt, qdrOpt, qdlOpt + 1, qdrOpt + 1);
  }
}
