#include <fs/fs.h>
#include <lib/synchronized.h>

namespace fs
{
    namespace DeviceManager
    {
        Synchronized<Vector<Node *>> devices;

        class DevFs : public Node
        {
        public:
            DevFs()
            {
                name = "dev";
                type = DIRECTORY;
            }

            Node *findEntry(const String &name)
            {
                return *devices.find([name](Node *device)
                                     { return device->name == name; });
            }

        private:
        };

        DevFs instance;

        void init()
        {
            new (&instance) DevFs();
            new (&devices) Synchronized<Vector<Node *>>();
            mountNode("/dev", &instance);
        }

        void registerDevice(Node *node)
        {
            devices.lock();
            devices.push(node);
            devices.unlock();
        }
    } // namespace DeviceManager

} // namespace fs
