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


#include "Simulator.hpp"
#include "Argtool.hpp"
#include <npm/World.hpp>
#include <npm/gfx/wrap_glut.hpp>
#include <sfl/gplan/TraversabilityMap.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include <signal.h>
#include <err.h>
#include <unistd.h>


using namespace npm;
using namespace sfl;
using namespace boost;
using namespace std;


class Parameters
{
public:
  Parameters():
    robot_config_filename("robots.config"),
    layout_config_filename("layout.config"),
    world_name(""),
    world_filename(""),
    world_from_trav(""),
    no_glut(false),
    fatal_warnings(false),
    dump(false),
    help(false)
  {
  }
  
  string robot_config_filename;
  string layout_config_filename;
  string world_name;
  string world_filename;
  string world_from_trav;
  bool no_glut;
  bool fatal_warnings;
  bool dump;
  bool help;
};


typedef map<int, AppWindow*> appwin_handle_t;
static appwin_handle_t appwin_handle;

static Parameters params;
static const unsigned int glut_timer_ms(50);
static const unsigned int timestep_usec(100000);
static shared_ptr<Simulator> simulator;

static void parse_options(int argc, char ** argv);
static void init_glut(int argc, char ** argv);
static void reshape(int width, int height);
static void draw();
static void keyboard(unsigned char key, int x, int y);
static void timer(int handle);
static void cleanup();
static void sighandle(int signum);


int main(int argc, char ** argv)
{
  atexit(cleanup);
  if(signal(SIGINT, sighandle) == SIG_ERR){
    perror("ERROR: signal(SIGINT) failed");
    exit(EXIT_FAILURE);
  }
  if(signal(SIGHUP, sighandle) == SIG_ERR){
    perror("ERROR: signal(SIGHUP) failed");
    exit(EXIT_FAILURE);
  }
  if(signal(SIGTERM, sighandle) == SIG_ERR){
    perror("ERROR: signal(SIGTERM) failed");
    exit(EXIT_FAILURE);
  }
  
  parse_options(argc, argv);
  
  shared_ptr<World> world;
  if( ! params.world_name.empty()){
    world = World::Create(params.world_name);
    if( ! world){
      cerr << "ERROR: invalid world name \"" << params.world_name << "\"\n";
      exit(EXIT_FAILURE);
    }
  }

  if( ! params.world_filename.empty()){
    ifstream is(params.world_filename.c_str());
    if( ! is){
      cerr << "ERROR: invalid world file \""
	   << params.world_filename << "\".\n";
      exit(EXIT_FAILURE);
    }
    world = World::Parse(is, &cerr);
    if( ! world){
      cerr << "ERROR: parsing world file \"" << params.world_filename
	   << "\" failed\n";
      exit(EXIT_FAILURE);
    }
  }
  
  if( ! params.world_from_trav.empty()){
    ifstream trav(params.world_from_trav.c_str());
    if( ! trav){
      cerr << "ERROR: invalid traversability file \""
	   << params.world_from_trav << "\".\n";
      exit(EXIT_FAILURE);
    }
    shared_ptr<TraversabilityMap>
      traversability(TraversabilityMap::Parse(trav, &cerr));
    if( ! traversability){
      cerr << "ERROR: parsing of traversability file \""
	   << params.world_from_trav << "\" failed.\n";
      exit(EXIT_FAILURE);
    }
    if( ! world)
      world.reset(new World(traversability->name));
    world->ApplyTraversability(*traversability);
  }
  
  if( ! world){
    cerr << "ERROR: no world creation method specified.\n"
	 << "  use one of -w, -W, or -M\n"
	 << "  see -h for more help\n";
    exit(EXIT_FAILURE);
  }
  
  simulator.
    reset(new Simulator(world, 0.000001 * timestep_usec,
			params.robot_config_filename,
			params.layout_config_filename,
			params.fatal_warnings));
  simulator->InitRobots();
  if (params.dump) {
    cout << "==================================================\n"
	 << "CAMERAS:\n";
    Instance<UniqueManager<Camera> >()->PrintCatalog(cout);
    cerr << "\n==================================================\n"
	 << "DRAWINGS:\n";
    Instance<UniqueManager<Drawing> >()->PrintCatalog(cout);
    exit(EXIT_SUCCESS);
  }
  simulator->Init();
  
  if(params.no_glut){
    simulator->SetContinuous();
    while(true){
      simulator->Idle();
      usleep(timestep_usec);
    }
  }
  else{
    init_glut(argc, argv);
    glutMainLoop();
  }
}


void init_glut(int argc, char ** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  
  for (size_t ii(0); ii < simulator->GetNAppWindows(); ++ii) {
    AppWindow * appwin(simulator->GetAppWindow(ii));
    int width, height;
    appwin->GetSize(width, height);
    appwin->InitLayout();
    
    glutInitWindowPosition(10 * ii, 10 * ii);
    glutInitWindowSize(width, height);
    
    int const handle(glutCreateWindow(appwin->name.c_str()));
    if (0 == handle)
      errx(EXIT_FAILURE, "init_glut(): glutCreateWindow(%s) failed for appwin %zu",
	   appwin->name.c_str(), ii);
    printf("appwin %zu: %s\n", ii, appwin->name.c_str());
    appwin_handle.insert(make_pair(handle, appwin));
    
    // all windows get draw, reshape, and keyboard functions
    glutDisplayFunc(draw);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    
    // simulator tick (wallclock) is on the first window only
    if (0 == ii)
      glutTimerFunc(glut_timer_ms, timer, handle);
  }
  
  if (appwin_handle.empty())
    errx(EXIT_FAILURE, "init_glut(): no application window");
}


void reshape(int width, int height)
{
  int const handle(glutGetWindow());
  appwin_handle_t::iterator ih(appwin_handle.find(handle));
  if (appwin_handle.end() == ih)
    errx(EXIT_FAILURE, "reshape(): non-registered GLUT handle");
  ih->second->Reshape(width, height);
}


void draw()
{
  for (appwin_handle_t::iterator ih(appwin_handle.begin()); ih != appwin_handle.end(); ++ih) {
    glutSetWindow(ih->first);
    glClear(GL_COLOR_BUFFER_BIT);
    ih->second->Draw();
    glFlush();
    glutSwapBuffers();
  }
}


void keyboard(unsigned char key,
	      int x,
	      int y)
{
  int const handle(glutGetWindow());
  appwin_handle_t::iterator ih(appwin_handle.find(handle));
  if (appwin_handle.end() == ih)
    errx(EXIT_FAILURE, "keyboard(): non-registered GLUT handle");
  ih->second->Keyboard(key, x, y);
  glutPostRedisplay();
}


void timer(int handle)
{
  if (simulator->Idle())
    for (appwin_handle_t::iterator ih(appwin_handle.begin()); ih != appwin_handle.end(); ++ih) {
      glutSetWindow(ih->first);
      glutPostRedisplay();
    }
  
  glutTimerFunc(glut_timer_ms, timer, handle);
}


void parse_options(int argc, char ** argv)
{
  cout << "command line:";
  for (int ii(0); ii < argc; ++ii)
    cout << " " << argv[ii];
  cout << "\n  use -h to display some help\n";
  
  typedef Argtool::Callback<string> StringCB;
  typedef Argtool::BoolCallback BoolCB;
  
  Argtool atl;
  atl.Add(new StringCB(params.layout_config_filename,
			 'l', "layout", "Name of the layout config file."));
  atl.Add(new StringCB(params.robot_config_filename,
			 'r', "robot", "Name of the robot config file."));
  atl.Add(new StringCB(params.world_name,
			 'w', "world", "Name of the world (expo|mini|tta)."));
  atl.Add(new StringCB(params.world_filename,
			 'W', "worldfile", "World file."));
  atl.Add(new StringCB(params.world_from_trav,
			 'M', "world-trav", "Add travmap lines to world."));
  atl.Add(new BoolCB(params.no_glut,
		       'n', "no-glut", "Disable graphic output."));
  atl.Add(new BoolCB(params.fatal_warnings,
		       'f', "fwarn", "Fatal warnings."));
  atl.Add(new BoolCB(params.dump,
		       'd', "dump", "Dump available drawings and cameras."));
  atl.Add(new BoolCB(params.help,
		       'h', "help", "Print a help message."));
  
  try {
    atl.Parse(argc, argv, 0);
  }
  catch(runtime_error e){
    cerr << "ERROR: parse_options() failed:\n  " << e.what() << "\n"
	 << "  use -h for help\n";
    exit(EXIT_FAILURE);
  }
  
  if (params.help) {
    atl.UsageMessage(cout, string(argv[0]) + " <options>");
    exit(EXIT_SUCCESS);
  }
}


void cleanup()
{
  simulator.reset();
  // XXXX to do: shouldn't GLUT resources be freed or so?
}


void sighandle(int signum)
{
  cerr << "signal " << signum << ": exit\n";
  exit(EXIT_SUCCESS);
}