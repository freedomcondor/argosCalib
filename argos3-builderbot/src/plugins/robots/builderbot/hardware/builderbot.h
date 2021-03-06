/**
 * @file <argos3/plugins/robots/builderbot/hardware/builderbot.h>
 *
 * @author Michael Allwright - <allsey87@gmail.com>
 */

#ifndef BUILDERBOT_H
#define BUILDERBOT_H

namespace argos {
   class CLuaController;
}

struct iio_context;
struct iio_device;

#include <argos3/core/hardware/sensor.h>
#include <argos3/core/hardware/actuator.h>
#include <argos3/core/utility/math/rng.h>
#include <argos3/core/utility/configuration/argos_configuration.h>

namespace argos {

   class CBuilderBot {

   public:
      static CBuilderBot& GetInstance() {
         static CBuilderBot cBuilderBot;
         return cBuilderBot;
      }

      void SetSignal(int n_signal) {
         m_bSignalRaised = true;
         m_nSignal = n_signal;
      }

      void Init(TConfigurationNode& t_tree,
                const std::string& str_controller_id);
      void Destroy();
      void InitFramework(TConfigurationNode& t_tree);
      void InitController(TConfigurationNode& t_tree,
                          const std::string& str_controller_id);
      void InitHardware();


      void Execute();

      iio_context* GetContext() {
         return m_psContext;
      }

      iio_device* GetActuatorUpdateTrigger() {
         return m_psActuatorUpdateTrigger;
      }

      iio_device* GetSensorUpdateTrigger() {
         return m_psSensorUpdateTrigger;
      }

      UInt32 GetTicksPerSec() {
         return m_unTicksPerSec;
      }

   private:

      CBuilderBot() :
         m_bSignalRaised(false),
         m_nSignal(0),
         m_pcRNG(nullptr),
         m_unTicksPerSec(0),
         m_pcController(nullptr),
         m_psContext(nullptr),
         m_psSensorUpdateTrigger(nullptr),
         m_psActuatorUpdateTrigger(nullptr) {}

      virtual ~CBuilderBot() {}

   private:
      /* signal handling variables */
      bool m_bSignalRaised;
      int m_nSignal;
      /* Pointer to the RNG */
      CRandom::CRNG* m_pcRNG;
      /* Target tick length for the controller */
      UInt32 m_unTicksPerSec;
      /*  Pointer to the controller */
      CLuaController* m_pcController;

      /* The vector of actuators */
      std::vector<CPhysicalActuator*> m_vecActuators;
      /* The vector of sensors */
      std::vector<CPhysicalSensor*> m_vecSensors;

      /* Triggers for updating the sensors and actuators */
      iio_context* m_psContext;
      iio_device* m_psSensorUpdateTrigger;
      iio_device* m_psActuatorUpdateTrigger;

      CPhysicalSensor* m_pcCamera;
   };

}

#endif
