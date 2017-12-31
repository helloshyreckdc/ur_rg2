#include "ur_modern_driver/ur_driver.h"
#include "ur_modern_driver/ur_hardware_interface.h"
#include "ur_modern_driver/do_output.h"
#include <string.h>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <time.h>
#include <cstdlib>

#include "ros/ros.h"
#include <ros/console.h>
#include "sensor_msgs/JointState.h"
#include "geometry_msgs/WrenchStamped.h"
#include "geometry_msgs/PoseStamped.h"
#include "control_msgs/FollowJointTrajectoryAction.h"
#include "actionlib/server/action_server.h"
#include "actionlib/server/server_goal_handle.h"
#include "trajectory_msgs/JointTrajectoryPoint.h"
#include "ur_msgs/SetIO.h"
#include "ur_msgs/SetPayload.h"
#include "ur_msgs/SetPayloadRequest.h"
#include "ur_msgs/SetPayloadResponse.h"
#include "ur_msgs/SetIORequest.h"
#include "ur_msgs/SetIOResponse.h"
#include "ur_msgs/IOStates.h"
#include "ur_msgs/Digital.h"
#include "ur_msgs/Analog.h"
#include "std_msgs/String.h"
#include "std_msgs/Float64.h"
#include "ur_control/RG2.h"
#include <controller_manager/controller_manager.h>
#include <realtime_tools/realtime_publisher.h>

/// TF
#include <tf/tf.h>
#include <tf/transform_broadcaster.h>

float width = 0; // store the target width
bool change = false; // call gripper service or not

void gripperCallback(const std_msgs::Float64::ConstPtr& target_width)
{
    if(width != target_width->data)
    {
		width = target_width->data;
		change = true;
    }
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "gripper_control");

    ros::NodeHandle n;
    ros::Subscriber sub = n.subscribe("gripper_width", 10, gripperCallback); // subscribe a topic which sends target width

    ros::Rate rate(1.0);
	
    while(ros::ok())
    {
		if(change)
		{
            ros::ServiceClient client = n.serviceClient<ur_control::RG2>("rg2_gripper/control_width");
            ur_control::RG2 srv;
            srv.request.target_width.data = width + 10; // this gripper program has something wrong, should compensate 1cm
            if (client.call(srv))
            {
		ros::param::set("finished_job", true);
                ROS_INFO("Succeed!");
            }
            else
            {
                ROS_ERROR("Failed to call service control_width");
                return 1;
            }
		    change = false;
		}
		ros::spinOnce();
		rate.sleep();

    }

    return 0;
}
