/**
 * @file <argos3/plugins/robots/builderbot/hardware/builderbot.cpp>
 *
 * @author Michael Allwright - <allsey87@gmail.com>
 */

#include "builderbot.h"

#include <iio.h>
#include <sys/time.h>
#include <unistd.h>

#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <list>
#include <thread>

#include <argos3/core/control_interface/ci_controller.h>
#include <argos3/core/hardware/actuator.h>
#include <argos3/core/hardware/sensor.h>
#include <argos3/core/utility/logging/argos_log.h>
#include <argos3/core/utility/plugins/factory.h>
#include <argos3/core/utility/rate.h>
#include <argos3/core/wrappers/lua/lua_controller.h>

#define ADD_TRIGGER_PATH "/sys/bus/iio/devices/iio_sysfs_trigger/add_trigger"
#define REMOVE_TRIGGER_PATH "/sys/bus/iio/devices/iio_sysfs_trigger/remove_trigger"

#define SENSOR_TRIGGER_IDX 0
#define ACTUATOR_TRIGGER_IDX 1

namespace argos {

   void CBuilderBot::Init(TConfigurationNode& t_tree, const std::string& str_controller_id) {
      /* Initialize the framework */
      InitFramework(GetNode(t_tree, "framework"));
      /* Initialize the controller */
      InitController(GetNode(t_tree, "controllers"), str_controller_id);
   }
   
   /****************************************/
   /****************************************/

   void CBuilderBot::Destroy() {
      /* delete actuators */
      for(CPhysicalActuator* pc_actuator : m_vecActuators)
         delete pc_actuator;
      /* delete sensors */
      for(CPhysicalSensor* pc_sensor : m_vecSensors)
         delete pc_sensor;
      /* delete the IIO library's context */
      iio_context_destroy(m_psContext);
      /* remove triggers */
      std::ofstream cRemoveTrigger;
      cRemoveTrigger.open(REMOVE_TRIGGER_PATH);
      cRemoveTrigger << std::to_string(SENSOR_TRIGGER_IDX) << std::flush;
      cRemoveTrigger.close();
      cRemoveTrigger.open(REMOVE_TRIGGER_PATH);
      cRemoveTrigger << std::to_string(ACTUATOR_TRIGGER_IDX) << std::flush;
      cRemoveTrigger.close();
      /* uninitialize the RNG */
      CRandom::RemoveCategory("argos");
      LOG << "[INFO] Controller terminated" << std::endl;
   }
  
   /****************************************/
   /****************************************/

   void CBuilderBot::InitFramework(TConfigurationNode& t_tree) {
      try {
         /* Get the experiment node */
         TConfigurationNode tExperiment = GetNode(t_tree, "experiment");
         /* Parse random seed */
         UInt32 unRandomSeed;
         GetNodeAttributeOrDefault(tExperiment,
                                   "random_seed",
                                   unRandomSeed,
                                   static_cast<UInt32>(0));
         /* If random seed is 0 or is not specified */
         if(unRandomSeed == 0) {
            /* Set the random seed based on the current clock time */
            struct timeval sTimeValue;
            ::gettimeofday(&sTimeValue, nullptr);
            unRandomSeed = static_cast<UInt32>(sTimeValue.tv_usec);
         }
         /* Create and initialize the RNG */
         CRandom::CreateCategory("argos", unRandomSeed);
         LOG << "[INFO] Using random seed = " << unRandomSeed << std::endl;
         m_pcRNG = CRandom::CreateRNG("argos");
         /* Set the target tick length */
         GetNodeAttribute(tExperiment,
                          "ticks_per_second",
                          m_unTicksPerSec);
         /* Create the triggers */
         std::ofstream cAddTrigger;
         cAddTrigger.open(ADD_TRIGGER_PATH);
         cAddTrigger << std::to_string(SENSOR_TRIGGER_IDX) << std::flush;
         cAddTrigger.close();
         cAddTrigger.open(ADD_TRIGGER_PATH);
         cAddTrigger << std::to_string(ACTUATOR_TRIGGER_IDX) << std::flush;
         cAddTrigger.close();
         /* Create a local context for the IIO library */
         m_psContext = iio_create_local_context();
         /* validate the sensor update trigger */
         std::string strSensorUpdateTrigger("sysfstrig" + std::to_string(SENSOR_TRIGGER_IDX));
         std::string strActuatorUpdateTrigger("sysfstrig" + std::to_string(ACTUATOR_TRIGGER_IDX));

         m_psSensorUpdateTrigger = 
            ::iio_context_find_device(m_psContext, strSensorUpdateTrigger.c_str());
         if(m_psSensorUpdateTrigger == nullptr) { 
            THROW_ARGOSEXCEPTION("Could not find IIO trigger \"" << strSensorUpdateTrigger << "\"");
         }
         if(!::iio_device_is_trigger(m_psSensorUpdateTrigger)) {
            THROW_ARGOSEXCEPTION("IIO device \"" << strSensorUpdateTrigger << "\" is not a trigger");
         }
         /* validate the sensor update trigger */
         m_psActuatorUpdateTrigger = 
            ::iio_context_find_device(m_psContext, strActuatorUpdateTrigger.c_str());
         if(m_psActuatorUpdateTrigger == nullptr) { 
            THROW_ARGOSEXCEPTION("Could not find IIO trigger \"" << strActuatorUpdateTrigger << "\"");
         }
         if(!::iio_device_is_trigger(m_psActuatorUpdateTrigger)) {
            THROW_ARGOSEXCEPTION("IIO device \"" << strActuatorUpdateTrigger << "\" is not a trigger");
         }
      }
      catch(CARGoSException& ex) {
         THROW_ARGOSEXCEPTION_NESTED("Failed to initialize framework", ex);
      }
   }

   /****************************************/
   /****************************************/

   void CBuilderBot::InitController(TConfigurationNode& t_tree, const std::string& str_controller_id) {
      try {
         std::string strControllerLabel;
         TConfigurationNodeIterator itController;
         for(itController = itController.begin(&t_tree);
             itController != itController.end();
             ++itController) {
            std::string strControllerId;
            GetNodeAttributeOrDefault(*itController, "id", strControllerId, strControllerId);
            if(str_controller_id.empty() || strControllerId == str_controller_id) {
               strControllerLabel = itController->Value();
               break;
            }
         }
         if(strControllerLabel.empty()) {
            THROW_ARGOSEXCEPTION("could not find controller in the experiment configuration file")
         }
         /* Create the controller */
         CCI_Controller* pcController = CFactory<CCI_Controller>::New(strControllerLabel);
         m_pcController = dynamic_cast<CLuaController*>(pcController);
         if(m_pcController == nullptr) {
            THROW_ARGOSEXCEPTION("ERROR: controller \"" << strControllerLabel << "\" is not a Lua controller");
         }
         std::string strImpl;
         /* Go through actuators */
         TConfigurationNode& tActuators = GetNode(*itController, "actuators");
         TConfigurationNodeIterator itAct;
         for(itAct = itAct.begin(&tActuators);
             itAct != itAct.end();
             ++itAct) {
            /* itAct->Value() is the name of the current actuator */
            GetNodeAttribute(*itAct, "implementation", strImpl);
            CPhysicalActuator* pcAct = CFactory<CPhysicalActuator>::New(itAct->Value() + " (" + strImpl + ")");
            CCI_Actuator* pcCIAct = dynamic_cast<CCI_Actuator*>(pcAct);
            if(pcCIAct == nullptr) {
               THROW_ARGOSEXCEPTION("BUG: actuator \"" << itAct->Value() << "\" does not inherit from CCI_Actuator");
            }
            pcCIAct->Init(*itAct);
            m_vecActuators.emplace_back(pcAct);
            m_pcController->AddActuator(itAct->Value(), pcCIAct);
         }
         /* Go through sensors */
         TConfigurationNode& tSensors = GetNode(*itController, "sensors");
         TConfigurationNodeIterator itSens;
         for(itSens = itSens.begin(&tSensors);
             itSens != itSens.end();
             ++itSens) {
            /* itSens->Value() is the name of the current actuator */
            GetNodeAttribute(*itSens, "implementation", strImpl);
            CPhysicalSensor* pcSens = CFactory<CPhysicalSensor>::New(itSens->Value() + " (" + strImpl + ")");
            CCI_Sensor* pcCISens = dynamic_cast<CCI_Sensor*>(pcSens);
            if(pcCISens == nullptr) {
               THROW_ARGOSEXCEPTION("BUG: sensor \"" << itSens->Value() << "\" does not inherit from CCI_Sensor");
            }
            pcCISens->Init(*itSens);
            if(itSens->Value() == "builderbot_cam") {
               m_pcCamera = pcSens;
            }
            else {
               m_vecSensors.emplace_back(pcSens);
            }
            m_pcController->AddSensor(itSens->Value(), pcCISens);
         }
         /* Set the controller id */
         char pchBuffer[32];
         if (::gethostname(pchBuffer, 32) == 0) {
            LOG << "[INFO] Setting controller id to hostname \""
                << pchBuffer << "\""
                << std::endl;
            m_pcController->SetId(pchBuffer);
         } 
         else {
            LOGERR << "[WARNING] Failed to get the hostname."
                   << "Setting controller id to \"builderbot\""
                   << std::endl;
            m_pcController->SetId("builderbot");
         }
         /* If the parameters node doesn't exist, create one */
         if(!NodeExists(*itController, "params")) {
            TConfigurationNode tParamsNode("params");
            AddChildNode(*itController, tParamsNode);
         }
         /* Init the controller with the parameters */
         m_pcController->Init(GetNode(*itController, "params"));
         /* check for errors */
         if(!m_pcController->IsOK()) {
            THROW_ARGOSEXCEPTION("Controller: " << m_pcController->GetErrorMessage());
         }
      }
      catch(CARGoSException& ex) {
         THROW_ARGOSEXCEPTION_NESTED("Failed to initialize framework", ex);
      }
   }

   /****************************************/
   /****************************************/

   void CBuilderBot::Execute() {
      /* initialize the tick rate */
      CRate cRate(m_unTicksPerSec);
      /* start the main control loop */
      try {
         for(;;) {
            /* start the camera update on a separate thread */
            //std::future<void> cCameraUpdate = 
            //   std::async(std::launch::async, std::bind(&CPhysicalSensor::Update, m_pcCamera));

            /* pull in samples from the sensor buffers */
            ::iio_device_attr_write_bool(m_psSensorUpdateTrigger, "trigger_now", true);
            /* update the sensors on this thread */
            for(CPhysicalSensor* pc_sensor : m_vecSensors) {
               pc_sensor->Update();
               if(m_bSignalRaised) {
                  THROW_ARGOSEXCEPTION("Signal " << m_nSignal << " raised during sensor update");
               }
            }
            /* wait for the camera update thread to complete */
            //cCameraUpdate.wait();
            /* forward any exceptions from the update thread to this thread */
            //cCameraUpdate.get();

            /* sleep if required */
            cRate.Sleep();
            /* step the internal state machine */
            m_pcController->ControlStep();
            /* check for errors */
            if(!m_pcController->IsOK()) {
               THROW_ARGOSEXCEPTION("Controller: " << m_pcController->GetErrorMessage());
            }
            /* actuator update */
            for(CPhysicalActuator* pc_actuator : m_vecActuators) {
               pc_actuator->Update();
               if(m_bSignalRaised) {
                  THROW_ARGOSEXCEPTION("Signal " << m_nSignal << " raised during actuator update");
               }
            }
            /* push actuator buffers to hardware */
            ::iio_device_attr_write_bool(m_psActuatorUpdateTrigger, "trigger_now", true);
            /* flush the logs */
            LOG.Flush();
            LOGERR.Flush();
         }
      }
      catch(CARGoSException& ex) {
         LOG.Flush();
         LOGERR << "[FATAL] Exception thrown in the control loop" << std::endl
                << ex.what() << std::endl;
         LOGERR.Flush();
         return;
      }
   }

   /****************************************/
   /****************************************/

}
