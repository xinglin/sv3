
#include <switch.hh>
#include <cstdarg>
#include <unistd.h>

namespace Switch {

  void Switch::logf(char const *str, ...)
  {
    va_list  ap;
    va_start(ap, str);
    printf("%16s: ", "switch");
    vprintf(str, ap);
    puts("");
    va_end(ap);
  }

  void Switch::loop()
  {
    const unsigned ms       = 1000;
    logf("Main loop entered. Polling every %ums.", ms);

    while (true) {
      for (Port *src_port : _ports) {
        logf("Polling port '%s'.", src_port->name());

        PacketJob *pj = src_port->poll();
        if (pj == nullptr) continue;

        auto &ehdr     = pj->ethernet_header();
        //logf("MAC %s hashes to %08x.", ehdr.dst.to_str(), Ethernet::hash(ehdr.dst));
        Port *dst_port = not ehdr.dst.is_multicast() ? _mac_table[ehdr.dst] : nullptr;
        assert(dst_port != src_port and
               dst_port != &_bcast_port);

        if (not (src_port == &_bcast_port or ehdr.src.is_multicast())) {
          if (_mac_table[ehdr.src] != src_port)
            logf("MAC %s owned by port '%s.", ehdr.src.to_str(),
                 src_port->name());

          _mac_table.add(ehdr.src, src_port);
        }

        if (UNLIKELY(!dst_port))
          dst_port = &_bcast_port;

        dst_port->receive(*src_port, *pj);
      }
      // XXX We should block here.
      usleep(1000*ms);
    }
  }

  void Switch::attach_port(Port &p)
  {
    _ports.push_front(&p);
    logf("Attaching Port '%s'. We have %u port%s.",
         p.name(), _ports.size(), _ports.size() == 1 ? "" : "s");
  }
}

// EOF