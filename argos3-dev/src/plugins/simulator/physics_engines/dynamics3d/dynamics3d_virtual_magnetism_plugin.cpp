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
      GetNodeAttributeOrDefault(t_tree, "g", m_fAcceleration, m_fAcceleration);
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

      std::cerr << "------- an update ---------------" << std::endl;

      for(std::vector< std::shared_ptr<CDynamics3DModel::CAbstractBody> >::iterator 
          itBody0 = std::begin(m_vecTargets);
          itBody0 != std::end(m_vecTargets) - 1;
          ++itBody0) {

         for(std::vector< std::shared_ptr<CDynamics3DModel::CAbstractBody> >::iterator 
             itBody1 = std::next(itBody0, 1);
             itBody1 != std::end(m_vecTargets);
             ++itBody1) {
            std::shared_ptr<CDynamics3DModel::CAbstractBody>& ptr_body0 = *itBody0;
            std::shared_ptr<CDynamics3DModel::CAbstractBody>& ptr_body1 = *itBody1;

            btVector3 btLocation0 = ptr_body0->GetTransform().getOrigin();
            btVector3 btLocation1 = ptr_body1->GetTransform().getOrigin();

            btQuaternion btQuaternion0 = ptr_body0->GetTransform().getRotation();
            btQuaternion btQuaternion1 = ptr_body1->GetTransform().getRotation();

            CBoxEntity* pcBoxEntity0 = 
               dynamic_cast<CBoxEntity*>(&ptr_body0->GetModel().GetComposableEntity());    
            CBoxEntity* pcBoxEntity1 = 
               dynamic_cast<CBoxEntity*>(&ptr_body1->GetModel().GetComposableEntity());    
            std::cerr << "I am a pair:: " << pcBoxEntity0->GetId() << ":"
                                          << btLocation0.getX() << ","
                                          << btLocation0.getY() << ","
                                          << btLocation0.getZ() << std::endl

                                          << pcBoxEntity1->GetId() << ":"
                                          << btLocation1.getX() << ","
                                          << btLocation1.getY() << ","
                                          << btLocation1.getZ() << std::endl;
         }
      }


      for(std::shared_ptr<CDynamics3DModel::CAbstractBody>& ptr_body : m_vecTargets) {
         btVector3 cGravity(-m_fAcceleration * ptr_body->GetData().Mass, 0.0, 0.0);
         ptr_body->ApplyForce(cGravity);

         /* draw debug lines */
         if(m_pcWidget != nullptr) {
            const btVector3& cEnd = ptr_body->GetTransform().getOrigin();

            m_pcWidget->m_vecDebugLines.emplace_back(
               CVector3(0,0,0),
               CVector3(cEnd.getX(), -cEnd.getZ(), cEnd.getY()),
               true,
               1.0f
            );
         }
      }
   }
   
   /****************************************/
   /****************************************/
   
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
