/**
 * @file <argos3/plugins/simulator/physics_engines/dynamics3d/dynamics3d_virtual_magnetism_plugin.cpp>
 *
 * @author Michael Allwright - <allsey87@gmail.com>
 */

#include "dynamics3d_virtual_magnetism_plugin.h"

#include <argos3/plugins/simulator/physics_engines/dynamics3d/dynamics3d_model.h>
#include <argos3/plugins/simulator/entities/box_entity.h>
#include <algorithm>
#include <vector>

namespace argos {
   
   /****************************************/
   /****************************************/
   
   void CDynamics3DVirtualMagnetismPlugin::Init(TConfigurationNode& t_tree) {
      //GetNodeAttributeOrDefault(t_tree, "g", m_fAcceleration, m_fAcceleration);
   } 

   /****************************************/
   /****************************************/

   void CDynamics3DVirtualMagnetismPlugin::RegisterModel(CDynamics3DModel& c_model) {
      CBoxEntity* pcBoxEntity = dynamic_cast<CBoxEntity*>(&c_model.GetComposableEntity());
      if(pcBoxEntity != nullptr) {
         std::cerr << "I am a box: " << pcBoxEntity->GetId() << std::endl;
         std::vector<std::shared_ptr<CDynamics3DModel::CAbstractBody> >& vecBodies = c_model.GetBodies();
         m_vecTargets.insert(std::end(m_vecTargets),
                             std::begin(vecBodies),
                             std::end(vecBodies));
      }
   }

   /****************************************/
   /****************************************/

   void CDynamics3DVirtualMagnetismPlugin::UnregisterModel(CDynamics3DModel& c_model) {
      std::vector<std::shared_ptr<CDynamics3DModel::CAbstractBody> >::iterator itRemove =
         std::remove_if(std::begin(m_vecTargets),
                        std::end(m_vecTargets),
                        [&c_model] (const std::shared_ptr<CDynamics3DModel::CAbstractBody>& ptr_body) {
                           return (&ptr_body->GetModel() == &c_model);
                        });
      m_vecTargets.erase(itRemove, std::end(m_vecTargets));
   }
   
   /****************************************/
   /****************************************/

   void CDynamics3DVirtualMagnetismPlugin::Update() {
      /* draw debug lines */
      if(m_pcWidget == nullptr) {
         CVisualization& cVisualization = CSimulator::GetInstance().GetVisualization();
         CQTOpenGLRender* pcQtOpenGLRender = dynamic_cast<CQTOpenGLRender*>(&cVisualization);
         if(pcQtOpenGLRender != nullptr) {
            m_pcWidget = &pcQtOpenGLRender->GetMainWindow().GetOpenGLWidget();
         }
      }
      else {
         m_pcWidget->m_vecDebugLines.clear();
      }

      for(std::vector< std::shared_ptr<CDynamics3DModel::CAbstractBody> >::iterator 
          itBody0 = std::begin(m_vecTargets);
          itBody0 != std::end(m_vecTargets) - 1;
          ++itBody0) {

         for(std::vector< std::shared_ptr<CDynamics3DModel::CAbstractBody> >::iterator 
             itBody1 = std::next(itBody0, 1);
             itBody1 != std::end(m_vecTargets);
             ++itBody1) {

            /* get body pointer */
            std::shared_ptr<CDynamics3DModel::CAbstractBody>& ptr_body0 = *itBody0;
            std::shared_ptr<CDynamics3DModel::CAbstractBody>& ptr_body1 = *itBody1;
            btScalar fBoxDistance = ptr_body0->GetTransform().getOrigin().distance(
                                    ptr_body1->GetTransform().getOrigin());

            /* debug message : print pair id */
            /*
            CBoxEntity* pcBoxEntity0 = 
               dynamic_cast<CBoxEntity*>(&(ptr_body0->GetModel().GetComposableEntity()));
            CBoxEntity* pcBoxEntity1 = 
               dynamic_cast<CBoxEntity*>(&(ptr_body1->GetModel().GetComposableEntity()));
            std::cerr << "a pair: " << std::endl;
            if(pcBoxEntity0 != nullptr) 
               std::cerr << "I am a box: " << pcBoxEntity0->GetId() << std::endl;
            if(pcBoxEntity1 != nullptr) 
               std::cerr << "I am a box: " << pcBoxEntity1->GetId() << std::endl;
            */

            /* calculate rotated offset */
            std::array<btVector3, 8> arrBody0MagnetRotatedOffsets;
            std::array<btVector3, 8> arrBody1MagnetRotatedOffsets;
            UInt32 unMagnetIndex = 0;
            for (const btVector3& c_offset : m_arrMagnetOffsets) {
               btTransform cMagnetRotatedOffset0(ptr_body0->GetTransform().getRotation(),
                                                 btVector3(0, 0, 0));
               btTransform cMagnetRotatedOffset1(ptr_body1->GetTransform().getRotation(),
                                                 btVector3(0, 0, 0));
               btTransform cOffset(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), c_offset);
               cMagnetRotatedOffset0 *= cOffset;
               cMagnetRotatedOffset1 *= cOffset;
               arrBody0MagnetRotatedOffsets[unMagnetIndex] = cMagnetRotatedOffset0.getOrigin();
               arrBody1MagnetRotatedOffsets[unMagnetIndex] = cMagnetRotatedOffset1.getOrigin();
               unMagnetIndex++;
            }

            /* calculate rotated Axis */
            std::array<btVector3, 6> arrBody0RotatedAxis;
            std::array<btVector3, 6> arrBody1RotatedAxis;
            UInt32 unAxisIndex = 0;
            for (const btVector3& c_axis: m_arrAxis) {
               btTransform cRotatedAxis0(ptr_body0->GetTransform().getRotation(),
                                         btVector3(0, 0, 0));
               btTransform cRotatedAxis1(ptr_body1->GetTransform().getRotation(),
                                         btVector3(0, 0, 0));
               btTransform cAxis(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), c_axis);
               cRotatedAxis0 *= cAxis;
               cRotatedAxis1 *= cAxis;
               arrBody0RotatedAxis[unAxisIndex] = cRotatedAxis0.getOrigin();
               arrBody1RotatedAxis[unAxisIndex] = cRotatedAxis1.getOrigin();

                     // draw rotated axis
                     /*
                     m_pcWidget->m_vecDebugLines.emplace_back(
                        CVector3( ptr_body0->GetTransform().getOrigin().getX(),
                                 -ptr_body0->GetTransform().getOrigin().getZ(),
                                  ptr_body0->GetTransform().getOrigin().getY()
                        ),
                       CVector3( ptr_body0->GetTransform().getOrigin().getX() +
                                 arrBody0RotatedAxis[unAxisIndex].getX(),
                                -ptr_body0->GetTransform().getOrigin().getZ() -
                                 arrBody0RotatedAxis[unAxisIndex].getZ(),
                                 ptr_body0->GetTransform().getOrigin().getY() +
                                 arrBody0RotatedAxis[unAxisIndex].getY()
                        ),
                        true,
                        1.0f
                     );
                     */

               unAxisIndex++;
            }


            /* calculate rotated location */
            std::array<btVector3, 8> arrBody0MagnetLocations;
            std::array<btVector3, 8> arrBody1MagnetLocations;
            for (unMagnetIndex = 0; unMagnetIndex < 8; unMagnetIndex++) {
               arrBody0MagnetLocations[unMagnetIndex] = ptr_body0->GetTransform().getOrigin() + 
                                                        arrBody0MagnetRotatedOffsets[unMagnetIndex];
               arrBody1MagnetLocations[unMagnetIndex] = ptr_body1->GetTransform().getOrigin() + 
                                                        arrBody1MagnetRotatedOffsets[unMagnetIndex];
            }

            /* apply force */
            UInt32 unMagnetIndex0 = 0;
            for (const btVector3& btMagnet0 : arrBody0MagnetLocations) {
               UInt32 unMagnetIndex1 = 0;
               for (const btVector3& btMagnet1: arrBody1MagnetLocations) {
                  /* distance and relative vector */
                  btScalar fDistance = btMagnet0.distance(btMagnet1);
                  btVector3 btDirMag0ToMag1 = (btMagnet1 - btMagnet0).normalize();
                  btVector3 btDirMag1ToMag0 = -btDirMag0ToMag1;

                  //if ((fDistance > 0.00910) && (fDistance < 0.03))
                  //if ((fDistance < 0.03) && (fDistance > 0.0005))
                  if (fDistance < 0.035)
                  {
                     /* force and torque for body0*/
                     btScalar fForce;
                     if (ptr_body0->GetData().Mass != 0)
                        fForce = m_fAcceleration * ptr_body0->GetData().Mass;
                     else
                        fForce = m_fAcceleration * ptr_body1->GetData().Mass;
                     //fForce /= btPow(fDistance, 1.5);
                     if (fBoxDistance < 0.06) fForce *= 3;

                     /*
                     btScalar near = 0.00999;
                     btScalar far = 0.010;
                     if (fDistance < near) fForce *= 2.0;
                     else if (fDistance < far) fForce *= 1.0+(fDistance-far)/(near-far)*1.0;
                     */

                     btScalar fAxisScale = 0;
                     for (btVector3& axis : arrBody1RotatedAxis)
                        if (btDirMag1ToMag0.dot(axis) > fAxisScale)
                           fAxisScale = btDirMag1ToMag0.dot(axis);
                     for (btVector3& axis : arrBody0RotatedAxis)
                        if (btDirMag0ToMag1.dot(axis) > fAxisScale)
                           fAxisScale = btDirMag0ToMag1.dot(axis);
                     fAxisScale -= 0.70;
                     if (fAxisScale < 0) fAxisScale = 0;
                     fAxisScale *= 2;
                     ptr_body0->ApplyForce(btDirMag0ToMag1 * fForce * fAxisScale,
                                           1.00 * arrBody0MagnetRotatedOffsets[unMagnetIndex0]);
                     /* force and torque for body1*/
                     ptr_body1->ApplyForce(btDirMag1ToMag0 * fForce * fAxisScale,
                                           1.00 * arrBody1MagnetRotatedOffsets[unMagnetIndex1]);

                     /* debug : draw force */
                     /*
                     m_pcWidget->m_vecDebugLines.emplace_back(
                        CVector3( arrBody0MagnetLocations[unMagnetIndex0].getX(),
                                 -arrBody0MagnetLocations[unMagnetIndex0].getZ(),
                                 arrBody0MagnetLocations[unMagnetIndex0].getY()
                        ),
                        CVector3( arrBody0MagnetLocations[unMagnetIndex0].getX() + 
                                    (0.06 * fForce* fAxisScale*btDirMag0ToMag1).getX(),
                                 -(arrBody0MagnetLocations[unMagnetIndex0].getZ() + 
                                    (0.06 * fForce* fAxisScale*btDirMag0ToMag1).getZ()),
                                  arrBody0MagnetLocations[unMagnetIndex0].getY() + 
                                    (0.06 * fForce* fAxisScale*btDirMag0ToMag1).getY()
                        ),
                        true,
                        1.0f
                     );

                     m_pcWidget->m_vecDebugLines.emplace_back(
                        CVector3( arrBody1MagnetLocations[unMagnetIndex1].getX(),
                                 -arrBody1MagnetLocations[unMagnetIndex1].getZ(),
                                  arrBody1MagnetLocations[unMagnetIndex1].getY()
                        ),
                        CVector3( arrBody1MagnetLocations[unMagnetIndex1].getX() + 
                                     (0.06 * fForce* fAxisScale*btDirMag1ToMag0).getX(),
                                 -(arrBody1MagnetLocations[unMagnetIndex1].getZ() + 
                                     (0.06 * fForce* fAxisScale*btDirMag1ToMag0).getZ()),
                                  arrBody1MagnetLocations[unMagnetIndex1].getY() + 
                                     (0.06 * fForce* fAxisScale*btDirMag1ToMag0).getY()
                        ),
                        true,
                        1.0f
                     );
                     */
                     /* debug end : draw force */
                  }
                  unMagnetIndex1++;
               }
               unMagnetIndex0++;
            }
         }
      }
   }
   
   /****************************************/
   /****************************************/

   /****************************************/
   /****************************************/

   const std::array<btVector3, 8> CDynamics3DVirtualMagnetismPlugin::m_arrMagnetOffsets {
      btVector3( 0.023, -0.023f, -0.023),  btVector3(-0.023, -0.023f, -0.023),
      btVector3(-0.023, -0.023f,  0.023),  btVector3( 0.023, -0.023f,  0.023),
      btVector3( 0.023,  0.023f, -0.023),  btVector3(-0.023,  0.023f, -0.023),
      btVector3(-0.023,  0.023f,  0.023),  btVector3( 0.023,  0.023f,  0.023),
   };

   const std::array<btVector3, 6> CDynamics3DVirtualMagnetismPlugin::m_arrAxis {
      btVector3( 1.000, 0.000f, 0.000),
      btVector3( 0.000, 1.000f, 0.000),
      btVector3( 0.000, 0.000f, 1.000),

      btVector3(-1.000, 0.000f, 0.000),
      btVector3( 0.000,-1.000f, 0.000),
      btVector3( 0.000, 0.000f,-1.000),
   };
   
   REGISTER_DYNAMICS3D_PLUGIN(CDynamics3DVirtualMagnetismPlugin,
                              "virtual_magnetism",
                              "Michael Allwright [allsey87@gmail.com]",
                              "1.0",
                              "Applies a gravity force to all bodies in the simulation",
                              "For a description on how to use this plugin, please consult the documentation\n"
                              "for the dynamics3d physics engine plugin",
                              "Usable");

   /****************************************/
   /****************************************/

}
