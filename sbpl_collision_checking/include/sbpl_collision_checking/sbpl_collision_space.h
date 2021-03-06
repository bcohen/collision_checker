/*
 * Copyright (c) 2011, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/** \author Benjamin Cohen */

#ifndef _SBPL_COLLISION_SPACE_
#define _SBPL_COLLISION_SPACE_

#include <ros/ros.h>
#include <vector>
#include <math.h>
#include <sbpl_manipulation_components/occupancy_grid.h>
#include <sbpl_manipulation_components/collision_checker.h>
#include <sbpl_collision_checking/sbpl_collision_model.h>
#include <sbpl_geometry_utils/Interpolator.h>
#include <sbpl_geometry_utils/Voxelizer.h>
#include <sbpl_geometry_utils/SphereEncloser.h>
#include <leatherman/bresenham.h>
#include <leatherman/utils.h>
#include <tf_conversions/tf_kdl.h>
#include <angles/angles.h>
#include <arm_navigation_msgs/CollisionObject.h>
#include <geometry_msgs/Point.h>

//OMPL for interpolation
#include <ompl/base/State.h>
#include <ompl/base/ScopedState.h>
#include <ompl/geometric/PathGeometric.h>
#include <ompl/base/spaces/RealVectorStateSpace.h>
#include <ompl/base/spaces/SO2StateSpace.h>

using namespace std;

namespace sbpl_arm_planner
{

class SBPLCollisionSpace : public sbpl_arm_planner::CollisionChecker
{
  public:

    SBPLCollisionSpace(sbpl_arm_planner::OccupancyGrid* grid);

    ~SBPLCollisionSpace(){};

    bool init(std::string group_name, std::string ns="");

    void setPadding(double padding);
   
    bool setPlanningScene(const arm_navigation_msgs::PlanningScene &scene);

    void setRobotState(const arm_navigation_msgs::RobotState &state);

    void setSphereGroupsForCollisionCheck(const std::vector<std::string> &group_names);

    void setInterpolationParams(bool use_ompl, int num_steps=10);

    void recomputeDistanceField();

    void enableNonDefaultGroupsToWorldCheck(bool enable);

    /** --------------- Collision Checking ----------- */
    bool checkCollision(const std::vector<double> &angles, bool verbose, bool visualize, double &dist); // multi-res
    bool checkCollision(const std::vector<double> &angles, bool low_res, bool verbose, bool visualize, double &dist);
    bool checkCollision(const std::vector<double> &angles, std::vector<std::vector<std::vector<KDL::Frame> > > &frames, bool low_res, bool verbose, bool visualize, double &dist);
    bool checkPathForCollision(const std::vector<double> &start, const std::vector<double> &end, bool verbose, int &path_length, int &num_checks, double &dist, std::vector<std::vector<double> > *path_out=NULL);
    bool checkPathForCollision(const std::vector<double> &start, const std::vector<double> &end, std::vector<std::vector<std::vector<KDL::Frame> > > &frames, bool verbose, int &path_length, int &num_checks, double &dist, std::vector<std::vector<double> > *path=NULL);

    bool checkSphereGroupAgainstWorld(const std::vector<double> &angles, Group *group, bool low_res, bool verbose, bool visualize, double &dist);
    bool checkSpheresAgainstWorld(const std::vector<std::vector<KDL::Frame> > &frames, const std::vector<Sphere*> &spheres, bool verbose, bool visualize, std::vector<KDL::Vector> &sph_poses, double &dist);
    bool checkSphereGroupAgainstSphereGroup(Group *group1, Group *group2, const std::vector<KDL::Vector> &spheres1, const std::vector<KDL::Vector> &spheres2, bool low_res1, bool low_res2, bool verbose, bool visualize, double &dist, int group1_min_priority, int group1_max_priority, int group2_min_priority, int group2_max_priority);

    inline bool isValidCell(const int x, const int y, const int z, const int radius);
    double isValidLineSegment(const std::vector<int> a, const std::vector<int> b, const int radius);
    bool getClearance(const std::vector<double> &angles, int num_spheres, double &avg_dist, double &min_dist);
    void getSpheresInCollision(std::vector<geometry_msgs::Point> &centers, std::vector<double> &radii);
    bool isStateValid(const std::vector<double> &angles, bool verbose, bool visualize, double &dist);
    bool isStateValid(const std::vector<double> &angles, std::vector<std::vector<std::vector<KDL::Frame> > > &frames, bool verbose, bool visualize, double &dist);
    bool isStateToStateValid(const std::vector<double> &angles0, const std::vector<double> &angles1, int &path_length, int &num_checks, double &dist, std::vector<std::vector<double> > *path_out=NULL);
    bool isStateToStateValid(const std::vector<double> &angles0, const std::vector<double> &angles1, std::vector<std::vector<std::vector<KDL::Frame> > > &frames, int &path_length, int &num_checks, double &dist, std::vector<std::vector<double> > *path_out=NULL);

    /** ---------------- Utils ---------------- */
    bool interpolatePath(const std::vector<double>& start, const std::vector<double>& end, std::vector<std::vector<double> >& path);
    bool interpolatePath(const std::vector<double>& start, const std::vector<double>& end, const std::vector<double>& inc, std::vector<std::vector<double> >& path);

    /** ------------ Kinematics ----------------- */
    std::string getGroupName() { return group_name_; };
    std::string getReferenceFrame() { return model_.getReferenceFrame(group_name_); };
    void setJointPosition(std::string name, double position);
    bool setPlanningJoints(const std::vector<std::string> &joint_names);
    bool getCollisionSpheres(const std::vector<double> &angles, Group *group, bool low_res, std::vector<std::vector<double> > &spheres);

    /* ------------- Collision Objects -------------- */
    void addCollisionObject(const arm_navigation_msgs::CollisionObject &object);
    void removeCollisionObject(const arm_navigation_msgs::CollisionObject &object);
    void processCollisionObjectMsg(const arm_navigation_msgs::CollisionObject &object);
    void removeAllCollisionObjects();
    void putCollisionObjectsInGrid();
    void getCollisionObjectVoxelPoses(std::vector<geometry_msgs::Pose> &points);
    
    /** --------------- Attached Objects -------------- */
    void attachObject(const arm_navigation_msgs::AttachedCollisionObject &obj);
    void attachSphere(std::string name, std::string link, geometry_msgs::Pose pose, double radius);
    void attachCylinder(std::string link, geometry_msgs::Pose pose, double radius, double length);
    void attachCube(std::string name, std::string link, geometry_msgs::Pose pose, double x_dim, double y_dim, double z_dim);
    void attachMesh(std::string name, std::string link, geometry_msgs::Pose pose, const std::vector<geometry_msgs::Point> &vertices, const std::vector<int> &triangles);
    void removeAttachedObject();
    bool getAttachedObject(const std::vector<double> &angles, bool low_res, std::vector<std::vector<double> > &xyz);
    bool setAttachedObjects(const std::vector<arm_navigation_msgs::AttachedCollisionObject> &objects);

    /** --------------- Debugging ---------------- */
    visualization_msgs::MarkerArray getVisualization(std::string type);
    visualization_msgs::MarkerArray getCollisionModelVisualization(const std::vector<double> &angles);
    visualization_msgs::MarkerArray getMeshModelVisualization(const std::string group_name, const std::vector<double> &angles);

    /** ------------- Self Collision ----------- */
    bool updateVoxelGroups();
    bool updateVoxelGroup(Group *g);
    bool updateVoxelGroup(std::string name);

    bool isObjectAttached() {return object_attached_;}; 

  private:

    sbpl_arm_planner::SBPLCollisionModel model_;
    sbpl_arm_planner::OccupancyGrid* grid_;

    /* ----------- Parameters ------------ */
    bool use_multi_level_collision_check_;
    bool check_nondefault_groups_against_world_;
    double padding_;
    double object_enclosing_sphere_radius_;
    std::string group_name_;

    /* ----------- Robot ------------ */
    std::vector<double> inc_;
    std::vector<double> min_limits_;
    std::vector<double> max_limits_;
    std::vector<bool> continuous_;

    /* ------------- Collision Objects -------------- */
    std::vector<std::string> known_objects_;
    std::map<std::string, arm_navigation_msgs::CollisionObject> object_map_;
    std::map<std::string, std::vector<Eigen::Vector3d> > object_voxel_map_;

    /** --------------- Attached Objects --------------*/
    bool object_attached_;
    int attached_object_segment_num_; 
    int attached_object_chain_num_;
    std::string attached_object_frame_;
    std::vector<Sphere> object_spheres_;
    std::vector<Sphere*> object_spheres_p_;  // hack
    std::map<std::string, std::vector<std::vector<double> > > object_spheres_map_;
    Group att_object_;

    double object_enclosing_low_res_sphere_radius_;
    std::vector<Sphere> low_res_object_spheres_;
    std::vector<Sphere*> low_res_object_spheres_p_;  // hack

    /** --------------- Interpolation --------------*/
    bool use_ompl_interpolation_;
    int num_interpolation_steps_;
    ompl::base::StateSpacePtr omplStateSpace_;
    ompl::base::SpaceInformationPtr si_;

    /* for debugging */
    std::vector<sbpl_arm_planner::Sphere> collision_spheres_;
};

inline bool SBPLCollisionSpace::isValidCell(const int x, const int y, const int z, const int radius)
{
  if(grid_->getCell(x,y,z) <= radius)
    return false;
  return true;
}

} 
#endif

