/*
 * Copyright (c) 2012, Willow Garage, Inc.
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
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
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

#include <OGRE/OgreCommon.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreVector3.h>

#include <rviz/display_context.h>
#include <rviz/display_factory.h>
#include <rviz/default_plugin/marker_display.h>
#include <rviz/ogre_helpers/arrow.h>
#include <rviz/ogre_helpers/billboard_line.h>

#include "ork_table_visual.h"

namespace object_recognition_ros
{
OrkTableVisual::OrkTableVisual(Ogre::SceneManager* scene_manager, Ogre::SceneNode* parent_node,
                               rviz::DisplayContext* display_context)
{
  scene_manager_ = scene_manager;

  // Here we create a node to store the pose of the Table header frame
  // relative to the RViz fixed frame.
  frame_node_ = parent_node->createChildSceneNode();
  object_node_ = frame_node_->createChildSceneNode();

  // Initialize the axes
  arrow_.reset(new rviz::Arrow(scene_manager_, object_node_));
  arrow_->setScale(Ogre::Vector3(0.1, 0.1, 0.1));

  // Initialize the name
  convex_hull_.reset(new rviz::BillboardLine(scene_manager_, object_node_));
  bounding_box_.reset(new rviz::BillboardLine(scene_manager_, object_node_));
}

OrkTableVisual::~OrkTableVisual()
{
  scene_manager_->destroySceneNode(object_node_);
  scene_manager_->destroySceneNode(frame_node_);
}

void
OrkTableVisual::setMessage(const object_recognition_msgs::Table& table, bool do_display_hull, bool do_display_bounding_box,
                           bool do_display_top)
{
  Ogre::Vector3 position(table.pose.position.x,
                         table.pose.position.y,
                         table.pose.position.z);
  object_node_->setOrientation(
    Ogre::Quaternion(table.pose.orientation.w,
                     table.pose.orientation.x,
                     table.pose.orientation.y,
                     table.pose.orientation.z));
  object_node_->setPosition(position);

  // Set the arrow on the object
  if (do_display_top) {
    arrow_->setScale(Ogre::Vector3(0.2, 0.2, 0.2));
    arrow_->setColor(0.0, 1.0, 1.0, 1.0);
    arrow_->setDirection(Ogre::Vector3(0, 0, 1));
  } else
    arrow_->setScale(Ogre::Vector3(0, 0, 0));

  //get the extents of the table
  float x_min = std::numeric_limits<float>::max(), x_max = -x_min;
  float y_min = std::numeric_limits<float>::max(), y_max = -y_min;

  for (size_t i = 0; i < table.convex_hull.size(); ++i) {
    if (table.convex_hull[i].x < x_min)
      x_min = table.convex_hull[i].x;
    if (table.convex_hull[i].x > x_max)
      x_max = table.convex_hull[i].x;
    if (table.convex_hull[i].y < y_min)
      y_min = table.convex_hull[i].y;
    if (table.convex_hull[i].y > y_max)
      y_max = table.convex_hull[i].y;
  }
  // Set the bounding box
  bounding_box_->clear();
  if (do_display_bounding_box) {
    bounding_box_->addPoint(Ogre::Vector3(x_min, y_min, 0));
    bounding_box_->addPoint(Ogre::Vector3(x_min, y_max, 0));
    bounding_box_->addPoint(Ogre::Vector3(x_max, y_max, 0));
    bounding_box_->addPoint(Ogre::Vector3(x_max, y_min, 0));
    bounding_box_->addPoint(Ogre::Vector3(x_min, y_min, 0));
    bounding_box_->setColor(1.0, 1.0, 0.0, 1.0);
    bounding_box_->setLineWidth(0.01);
  }

  // Set the convex hull
  convex_hull_->clear();
  if (do_display_hull) {
    for (size_t i = 0; i < table.convex_hull.size(); ++i)
      convex_hull_->addPoint(Ogre::Vector3(table.convex_hull[i].x, table.convex_hull[i].y, 0));
    convex_hull_->addPoint(Ogre::Vector3(table.convex_hull[0].x, table.convex_hull[0].y, 0));
    convex_hull_->setColor(0.0, 1.0, 1.0, 1.0);
    convex_hull_->setLineWidth(0.01);
  }
}

// Position and orientation are passed through to the SceneNode.
void
OrkTableVisual::setFramePosition(const Ogre::Vector3& position)
{
  frame_node_->setPosition(position);
}

void
OrkTableVisual::setFrameOrientation(const Ogre::Quaternion& orientation)
{
  frame_node_->setOrientation(orientation);
}
}    // end namespace object_recognition_ros
