/*
 * ClusterIPv4Range.cc
 *
 *  Created on: Aug 20, 2015
 *      Author: edgard
 */

#include "ClusterIPv4Range.hh"
#include "ProtoVEBClient.hh"

#include <string>
#include "ProtoVEBk.hh"

ClusterIPv4Range::ClusterIPv4Range(uint8_t k_parent) :
k_parent_(k_parent), has_hacked_(false), hacked_index_(0) {
}

ClusterIPv4Range::~ClusterIPv4Range() {
  // TODO Auto-generated destructor stub
}

// sed -i "s/\,/\\t/g" dveb_ismember_*

//#define ifrog "172.16.128.2"  // 16G

#define ifrog      "192.168.1.1"   // 16G
#define ferrari    "192.168.1.3"   // 16G
#define lamborgine "192.168.1.5"   // 24G
#define mustang    "192.168.1.7"   // 4G
#define bugatti    "192.168.1.9"   // 32G
#define maserati   "192.168.1.11"  // 32G
#define nvidia     "192.168.1.13"  // 8G
#define camaro     "192.168.1.15"  // 6G

// ./DvEB -v1 -mperformance -c55 -tk -u64 -f1 -l27 -o-1 -gcluster_seq | tee ~/dveb_output.txt
// ./DvEB -v1 -mperformance -c43 -tk -u64 -f1 -l27 -o-1  -gcluster_seq | tee ~/dveb_output.txt

// tmux attach-session -t0

// sub-total = 78/2 = 39
// pibic 9 * 2 = 18
// 18 + 39 = 57 clusters

// 16 + 32 + 32 + 6 = 86 GB
// 86 / 2 = 43 clusters

#define fusca "172.16.128."

// tar -czf DvEB.tgz DvEB/ DvEBserver/

// tmux
//
// tmux ls - listas as sessoes
// tmux attach-session -t <session name> - abre a sessão especificada.
//
// tecla chave = CTRL+B
//
// n - proxima janela
// p - janela anterior
// [ - entra modo scroll (q sai de modo scroll)
// c - nova janela
// d- sai do tmux sem terminar a sessao
// ?- lista os atalhos
// ,- renomear janela


// killall DvEBserver ; sleep 1 ; killall -9 DvEBserver ; rm -rf workspace ; rm -f workspace.tgz ; tar -xzf workspace.tgz && rm -rf build && mkdir build && cd build/ && cmake ../workspace/DvEBserver/ && make -j8 && cd && rm -rf workspace && rm -f workspace.tgz

// killall DvEBserver ; sleep 1 ; killall -9 DvEBserver ; rm -rf DvEB ; rm -rf DvEBserver ; rm -f DvEB.tgz ; scp 192.168.1.1:./DvEB.tgz . && tar -xzf DvEB.tgz && rm -rf build && mkdir build && cd build/ && cmake ../DvEBserver/ && make -j8 && cd && rm -rf DvEB && rm -rf DvEBserver && rm -f DvEB.tgz


// rm -rf workspace ; rm -f workspace.tgz ; rm -rf build/ ; scp edgard@172.16.128.172:/home/edgard/Projects/mestrado/DvEB/workspace.tgz . ; tar -xzf workspace.tgz ; mkdir build ; cd build ; cmake ../workspace/DvEBserver/ ; make -j8 ; cp ../workspace/DvEBserver/DvEBserver.sh . ;  rm -rf ../workspace ; rm -f ../workspace.tgz ; /etc/init.d/gdm stop ; /etc/init.d/lightdm stop ; /etc/init.d/gdm3 stop ; /etc/init.d/x11-common stop ; swapoff -a ; sh -c sync ; echo 3 > /proc/sys/vm/drop_caches ; free

// rm -rf workspace ; rm -f workspace.tgz ; rm -rf build/ ; scp edgard@172.16.128.172:/home/edgard/Projects/mestrado/DvEB/workspace.tgz . ; tar -xzf workspace.tgz ; mkdir build ; cd build ; cmake ../workspace/DvEBserver/ ; make -j8 ; cp ../workspace/DvEBserver/DvEBserver.sh . ;  rm -rf ../workspace ; rm -f ../workspace.tgz ; sudo /etc/init.d/gdm stop ; sudo /etc/init.d/lightdm stop ; sudo /etc/init.d/gdm3 stop ; sudo /etc/init.d/x11-common stop ; sudo /sbin/swapoff -a ; sudo sh -c sync ; sudo sh -c "echo 3 > /proc/sys/vm/drop_caches" ; free

// rm -rf workspace ; rm -f workspace.tgz ; rm -rf build/ ; scp edgard@172.16.128.172:/home/edgard/Projects/mestrado/DvEB/workspace.tgz . ; tar -xzf workspace.tgz ; mkdir build ; cd build ; cmake ../workspace/DvEBserver/ ; make -j8 ; cp ../workspace/DvEBserver/DvEBserver.sh . ;  /etc/init.d/gdm stop ; /etc/init.d/lightdm stop ; /etc/init.d/gdm3 stop ; /etc/init.d/x11-common stop ; swapoff -a ; sh -c sync ; echo 3 > /proc/sys/vm/drop_caches ; free


// /etc/init.d/gdm stop ; /etc/init.d/lightdm stop ; /etc/init.d/gdm3 stop ; /etc/init.d/x11-common stop ; swapoff -a ; sh -c sync ; echo 3 > /proc/sys/vm/drop_caches ; free

#define pibic01 "172.16.128.207"
#define pibic02 "172.16.130.226"  // usando metade da memoria (descobri, a compilacao foi 32 bits)
#define pibic03 "172.16.131.88"
#define pibic04 "172.16.128.224"
#define pibic05 "172.16.131.101"
#define pibic06 "172.16.131.84"
#define pibic07 "172.16.128.209"  // usando metade da memoria
#define pibic08 "172.16.130.82"
#define pibic09 "172.16.130.223"

struct IpPort {
  const char * ip;
  uint16_t port;
};

/*
static const IpPort ip_port_array_[] = {
    { "127.0.0.1", 8084 },
    { "127.0.0.1", 8085 },
    { "127.0.0.1", 8086 },
    { "127.0.0.1", 8087 },
    { "127.0.0.1", 8088 },
    { "127.0.0.1", 8089 }
};
*/

static const IpPort ip_port_array_[] = {

    { ferrari, 8083 },
    { ferrari, 8084 },
    { ferrari, 8085 },
    { ferrari, 8086 },
    { ferrari, 8087 },
    { ferrari, 8088 },
    { ferrari, 8089 },
    { ferrari, 8090 },

	/*
    { lamborgine, 8083 },
    { lamborgine, 8084 },
    { lamborgine, 8085 },
    { lamborgine, 8086 },
    { lamborgine, 8087 },
    { lamborgine, 8088 },
    { lamborgine, 8089 },
    { lamborgine, 8090 },
    { lamborgine, 8091 },
    { lamborgine, 8092 },
    { lamborgine, 8093 },
    { lamborgine, 8094 },
    */

//    { mustang, 8084 },
//    { mustang, 8085 },

    { bugatti, 8083 },
    { bugatti, 8084 },
    { bugatti, 8085 },
    { bugatti, 8086 },
    { bugatti, 8087 },
    { bugatti, 8088 },
    { bugatti, 8089 },
    { bugatti, 8090 },
    { bugatti, 8091 },
    { bugatti, 8092 },
    { bugatti, 8093 },
    { bugatti, 8094 },
    { bugatti, 8095 },
    { bugatti, 8096 },
    { bugatti, 8097 },
    { bugatti, 8098 },

    { maserati, 8083 },
    { maserati, 8084 },
    { maserati, 8085 },
    { maserati, 8086 },
    { maserati, 8087 },
    { maserati, 8088 },
    { maserati, 8089 },
    { maserati, 8090 },
    { maserati, 8091 },
    { maserati, 8092 },
    { maserati, 8093 },
    { maserati, 8094 },
    { maserati, 8095 },
    { maserati, 8096 },
    { maserati, 8097 },
    { maserati, 8098 },

    { camaro, 8083 },
    { camaro, 8084 },
    { camaro, 8085 },


  //  { fusca, 8083 },
  //  { fusca, 8084 },

    /*
    { pibic01, 8083 },
    { pibic01, 8084 },

    { pibic02, 8083 },
    { pibic02, 8084 },

    { pibic03, 8083 },
    { pibic03, 8084 },

    { pibic04, 8083 },
    { pibic04, 8084 },

    { pibic05, 8083 },
    { pibic05, 8084 },

    { pibic06, 8083 },
    { pibic06, 8084 },

    { pibic07, 8083 },
    { pibic07, 8084 },

    { pibic08, 8083 },
    { pibic08, 8084 },

    { pibic09, 8083 },
    { pibic09, 8084 }
*/
};


const std::shared_ptr<ProtoVEB> ClusterIPv4Range::operator [](uint64_t index) {
  if (index > ProtoVEB::k_properties[k_parent_].sqrt_high_u) {
    std::cerr << "Out of range" << std::endl;
    return std::shared_ptr<ProtoVEB>();
  }

  if (has_hacked_ && index == hacked_index_) {
    return hacked_pVEB_;
  }

  if (index >= sizeof(ip_port_array_)/sizeof(ip_port_array_[0])) {
    throw std::runtime_error("ClusterIPv4Range don't know any more machine nodes. index =" + std::to_string(index));
  }

  std::string address;
  uint16_t port;

  address = ip_port_array_[index].ip;
  port = ip_port_array_[index].port;

  std::shared_ptr<ProtoVEBClient> p(new ProtoVEBClient(address, std::to_string(port)));
  return std::dynamic_pointer_cast<ProtoVEB>(p);
}

void ClusterIPv4Range::set(uint64_t index, std::shared_ptr<ProtoVEB> pVEB) {
  throw std::runtime_error("ClusterIPv4Range can't be set.");
}

void ClusterIPv4Range::setHack(uint64_t index, std::shared_ptr<ProtoVEB> pVEB) {
  has_hacked_ = true;
  hacked_index_ = index;
  hacked_pVEB_ = pVEB;
}

std::string ClusterIPv4Range::status() {
	std::string st;
	for (uint64_t i = 0; i < sizeof(ip_port_array_) / sizeof(ip_port_array_[0]); i++) {
		if (i > 0)
			if (ip_port_array_[i].ip == ip_port_array_[i-1].ip)
				continue;

		st += std::string(ip_port_array_[i].ip) + ":" + std::to_string(ip_port_array_[i].port) + "{";
		std::shared_ptr<ProtoVEBClient> p(
				new ProtoVEBClient(ip_port_array_[i].ip, std::to_string(ip_port_array_[i].port)));
		st += p->status();
		st += "}\n";
	}
	return st;
}
