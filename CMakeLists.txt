cmake_minimum_required(VERSION 3.8)
project(uclv_rh8d_force_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(uclv_seed_robotics_ros_interfaces REQUIRED)
find_package(std_srvs REQUIRED)
find_package(uclv_dynamixel_utils REQUIRED)

add_executable(euler_integrator_controller src/euler_integrator_controller_node.cpp)
ament_target_dependencies(
  euler_integrator_controller
  "rclcpp"
  "uclv_seed_robotics_ros_interfaces" # std_msgs sta quì
  "std_srvs"
)

add_executable(proportional_controller src/proportional_controller_node.cpp)
ament_target_dependencies(
  proportional_controller
  "rclcpp"
  "uclv_seed_robotics_ros_interfaces" # std_msgs sta quì
  "std_srvs"

)

add_executable(force_norm src/force_norm_node.cpp)
ament_target_dependencies(
  force_norm
  "rclcpp"
  "uclv_seed_robotics_ros_interfaces" # std_msgs sta quì
)

add_executable(close src/close_node.cpp)
ament_target_dependencies(
  close
  "rclcpp"
  "uclv_seed_robotics_ros_interfaces" # std_msgs sta quì
  "std_srvs"
)

add_executable(open src/open_node.cpp)
ament_target_dependencies(
  open
  "rclcpp"
  "uclv_seed_robotics_ros_interfaces" # std_msgs sta quì
  "std_srvs"
)


add_executable(slipping_avoidance src/slipping_avoidance_node.cpp)
ament_target_dependencies(
  slipping_avoidance
  "rclcpp"
  "uclv_seed_robotics_ros_interfaces" # std_msgs sta quì
  "std_srvs"
)









install(TARGETS 
  euler_integrator_controller
  proportional_controller
  force_norm
  close
  open
  slipping_avoidance
  DESTINATION 
    lib/${PROJECT_NAME}
)


  

  

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
