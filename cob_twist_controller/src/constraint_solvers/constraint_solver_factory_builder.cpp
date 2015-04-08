/*
 *****************************************************************
 * \file
 *
 * \note
 *   Copyright (c) 2015 \n
 *   Fraunhofer Institute for Manufacturing Engineering
 *   and Automation (IPA) \n\n
 *
 *****************************************************************
 *
 * \note
 *   Project name: care-o-bot
 * \note
 *   ROS stack name: cob_control
 * \note
 *   ROS package name: cob_twist_controller
 *
 * \author
 *   Author: Marco Bezzon, email: Marco.Bezzon@ipa.fraunhofer.de
 *
 * \date Date of creation: March, 2015
 *
 * \brief
 *   This header contains the description of a class providing a static method to create constraint solver factory objects.
 *
 ****************************************************************/

#include "ros/ros.h"
#include <Eigen/Core>
#include <Eigen/SVD>
#include <kdl/jntarray.hpp>
#include "cob_twist_controller/augmented_solver_data_types.h"
#include "cob_twist_controller/constraint_solvers/constraint_solver_factory_builder.h"
#include "cob_twist_controller/damping_methods/damping_builder.h"
#include "cob_twist_controller/constraint_solvers/factories/solver_factory.h"
#include "cob_twist_controller/constraint_solvers/factories/joint_limit_avoidance_solver_factory.h"
#include "cob_twist_controller/constraint_solvers/factories/unconstraint_solver_factory.h"

/**
 * Out of the parameters generates a damping method (e.g. constant or manipulability) and calculates the damping factor.
 * Dependent on JLA active flag a JointLimitAvoidanceSolver or a UnconstraintSolver is generated to solve the IK problem.
 * The objects are generated for each solve-request. After calculation the objects are deleted.
 */
Eigen::MatrixXd ConstraintSolverFactoryBuilder::calculateJointVelocities(AugmentedSolverParams &asSolverParams,
                                                                        Matrix6Xd &jacobianData,
                                                                        Eigen::Transpose<Matrix6Xd> &jacobianDataTransposed,
                                                                        const Eigen::VectorXd &inCartVelocities,
                                                                        const KDL::JntArray& q,
                                                                        const KDL::JntArray& last_q_dot)
{
    DampingBase* db = DampingBuilder::create_damping(asSolverParams, jacobianData);
    double dampingFactor;
    if(NULL != db)
    {
        dampingFactor = db->get_damping_factor();
    }
    else
    {
        ROS_ERROR("Returning NULL factory due to damping creation error.");
        return Eigen::MatrixXd();
    }

    SolverFactory* sf = NULL;
    if (asSolverParams.JLA_active)
    {
        sf = new JointLimitAvoidanceSolverFactory();
    }
    else
    {
        sf = new UnconstraintSolverFactory();
    }
    Eigen::MatrixXd jntVel = sf->calculateJointVelocities(asSolverParams, jacobianData, jacobianDataTransposed, inCartVelocities, q, last_q_dot, dampingFactor);

    delete db;
    db = NULL;

    delete sf;
    sf = NULL;

    return jntVel;
}
