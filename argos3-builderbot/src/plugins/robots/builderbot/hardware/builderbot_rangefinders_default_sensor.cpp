/**
 * @file <argos3/plugins/robots/builderbot/hardware/builderbot_rangefinders_default_sensor.cpp>
 *
 * @author Michael Allwright - <allsey87@gmail.com>
 */

#include "builderbot_rangefinders_default_sensor.h"

#include <iio.h>
#include <sys/stat.h>

namespace argos {

   /****************************************/
   /****************************************/

   CBuilderBotRangefindersDefaultSensor::CBuilderBotRangefindersDefaultSensor() {}

   /****************************************/
   /****************************************/

   CBuilderBotRangefindersDefaultSensor::~CBuilderBotRangefindersDefaultSensor() {
      for(SPhysicalInterface& s_physical_interface : m_vecPhysicalInterfaces) {
         ::iio_buffer_destroy(s_physical_interface.Buffer);
         ::iio_device_set_trigger(s_physical_interface.Device, nullptr);
         ::iio_channel_disable(s_physical_interface.ProximityChannel);
         ::iio_channel_disable(s_physical_interface.IlluminanceChannel);
      }
   }

   /****************************************/
   /****************************************/

   void CBuilderBotRangefindersDefaultSensor::Init(TConfigurationNode& t_tree) {
      try {
         CCI_BuilderBotRangefindersSensor::Init(t_tree);
         /* Get context and trigger */
         iio_context* psContext = CBuilderBot::GetInstance().GetContext();
         iio_device* psUpdateTrigger = CBuilderBot::GetInstance().GetSensorUpdateTrigger();
         /* Parse the device name and channel names */
         std::string strDevice, strLeft, strRight;
         GetNodeAttribute(t_tree, "device", strDevice);
         /* loop over the devices */
         UInt32 unNumDevices = iio_context_get_devices_count(psContext);
         for(UInt32 un_index = 0; un_index < unNumDevices; un_index++) {
            iio_device* psDevice = iio_context_get_device(psContext, un_index);
            if(iio_device_get_name(psDevice) && iio_device_get_name(psDevice) == strDevice) {
               char pchBuffer[32];
               int ret = iio_device_attr_read(psDevice, "label", pchBuffer, sizeof(pchBuffer));
               if(ret < 0) {
                  THROW_ARGOSEXCEPTION("Could not read the IIO attribute \"label\" for device \"" << strDevice << "\"");
               }
               iio_channel* psProximity = ::iio_device_find_channel(psDevice, "proximity", false);
               if(psProximity == nullptr) {
                  THROW_ARGOSEXCEPTION("Could not find IIO channel \"proximity\" for device \"" << strDevice << "\"");
               }
               iio_channel* psIlluminance = ::iio_device_find_channel(psDevice, "illuminance", false);
               if(psIlluminance == nullptr) {
                  THROW_ARGOSEXCEPTION("Could not find IIO channel \"illuminance\" for device \"" << strDevice << "\"");
               }
               /* enable channels */
               ::iio_channel_enable(psProximity);
               ::iio_channel_enable(psIlluminance);
               /* set trigger */
               ::iio_device_set_trigger(psDevice, psUpdateTrigger);
               /* create buffer */
               iio_buffer* psBuffer = ::iio_device_create_buffer(psDevice, 1, false);
               if(psBuffer == nullptr) {
                  ::iio_channel_disable(psIlluminance);
                  ::iio_channel_disable(psProximity);
                  ::iio_device_set_trigger(psDevice, nullptr);
                  THROW_ARGOSEXCEPTION("Could not create IIO buffer: " << std::to_string(errno));
               }
               m_vecPhysicalInterfaces.emplace_back(pchBuffer, psDevice, psProximity, psIlluminance, psBuffer);
            }
         }
         /* add pointers to the control interface */
         for(SPhysicalInterface& s_physical_interface : m_vecPhysicalInterfaces) {
            m_vecInterfaces.push_back(&s_physical_interface);
         }
      }
      catch(CARGoSException& ex) {
         THROW_ARGOSEXCEPTION_NESTED("Initialization error in the BuilderBot rangefinders sensor.", ex);
      }
   }

   /****************************************/
   /****************************************/
   
   void CBuilderBotRangefindersDefaultSensor::Update() {
      UInt16 unIlluminanceRaw, unProximityRaw;
      for(SPhysicalInterface& s_physical_interface : m_vecPhysicalInterfaces) {
         ::iio_buffer_refill(s_physical_interface.Buffer);
         ::iio_channel_read(s_physical_interface.ProximityChannel,
                            s_physical_interface.Buffer,
                            &unProximityRaw, 2);
         ::iio_channel_read(s_physical_interface.IlluminanceChannel,
                            s_physical_interface.Buffer,
                            &unIlluminanceRaw, 2);
         /* convert samples to metric units */
         s_physical_interface.Proximity = ConvertToMeters(unProximityRaw);
         s_physical_interface.Illuminance = ConvertToLux(unIlluminanceRaw);
      }
   }



   /****************************************/
   /****************************************/

   void CBuilderBotRangefindersDefaultSensor::Reset() {
      for(SPhysicalInterface& s_physical_interface : m_vecPhysicalInterfaces) {
         s_physical_interface.Proximity = Real(0);
         s_physical_interface.Illuminance = Real(0);
      }
   }

   /****************************************/
   /****************************************/

   REGISTER_SENSOR(CBuilderBotRangefindersDefaultSensor,
                   "builderbot_rangefinders", "default",
                   "Michael Allwright [allsey87@gmail.com]",
                   "1.0",
                   "The BuilderBot rangefinders sensor.",
                   "To do\n\n",
                   "Under development"
   );

   /****************************************/
   /****************************************/


}
