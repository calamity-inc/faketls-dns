#include <iostream>

#include <dnsServerService.hpp>
#include <Server.hpp>
#include <ServerWebService.hpp>
#include <string.hpp>
#include <UniquePtr.hpp>

using namespace soup;

int main()
{
	Server serv;

	dnsServerService dns_srv([](const std::string& name, const dnsName&, const SocketAddr&, dnsType) -> std::vector<SharedPtr<dnsRecord>>
	{
		std::cout << "Query for " << name << std::endl; // TODO: Remove me once networking is confirmed working on all nodes

		std::vector<SharedPtr<dnsRecord>> res;
		if (name == "faketls.com")
		{
			res.emplace_back(soup::make_shared<dnsARecord>(name, 69420, Endianness::toNetwork(SOUP_IPV4(198, 251, 90, 234))));
			res.emplace_back(soup::make_shared<dnsTxtRecord>(name, 69420, "protonmail-verification=19b439bfab3f4c1b2811c7616f71deab93327ae5"));
			res.emplace_back(soup::make_shared<dnsMxRecord>(name, 69420, 10, "mail.protonmail.ch"));
			res.emplace_back(soup::make_shared<dnsMxRecord>(name, 69420, 20, "mailsec.protonmail.ch"));
		}
		else if (name == "www.faketls.com" || name == "ns1.faketls.com" || name == "ns2.faketls.com"
			|| name == "viatls.com" || name == "www.viatls.com" || name == "ns1.viatls.com" || name == "ns2.viatls.com"
			|| name == "p2ptls.com" || name == "www.p2ptls.com" || name == "ns1.p2ptls.com" || name == "ns2.p2ptls.com"
			)
		{
			res.emplace_back(soup::make_shared<dnsARecord>(name, 69420, Endianness::toNetwork(SOUP_IPV4(198, 251, 90, 234))));
		}
		/*else if (name == "_e4d5e9c5ec515aaf0b06041edeb568a7.viatls.com")
		{
			res.emplace_back(soup::make_shared<dnsCnameRecord>(name, 69420, "BE77722D1C1E6450820390BFE0EC7BCC.B91849E7453310D049502BF46A984609.66ad56ffd0888.comodoca.com"));
		}*/
		else
		{
			std::string str = name;
			string::replaceAll(str, ".faketls.com", "");
			string::replaceAll(str, ".viatls.com", "");
			string::replaceAll(str, ".p2ptls.com", "");
			string::replaceAll(str, "-", ".");
			string::replaceAll(str, "_", ":");
			IpAddr addr;
			if (addr.fromString(str))
			{
				if (addr.isV4())
				{
					res.emplace_back(soup::make_shared<dnsARecord>(name, 69420, addr.getV4()));
				}
				else
				{
					res.emplace_back(soup::make_shared<dnsAaaaRecord>(name, 69420, addr));
				}
			}
		}
		return res;
	});

#ifdef DOCKER
	serv.bindUdp(53, &dns_srv);
	std::cout << "Bound to UDP/53" << std::endl;
#else
	IpAddr addr;
	addr.fromString("198.251.90.234");
	serv.bindUdp(addr, 53, &dns_srv);
	std::cout << "Bound to UDP/" << addr.toString() << ":53" << std::endl;
#endif

	soup::ServerWebService web_srv{
		[](soup::Socket& s, soup::HttpRequest&& req, soup::ServerWebService&)
		{
			soup::ServerWebService::disableKeepAlive(s);
			soup::ServerWebService::sendText(s, ":)");
		}
	};
	if (serv.bind(4269, &web_srv))
	{
		std::cout << "Bound to TCP/4269" << std::endl;
	}
	else
	{
		std::cout << "FAILED TO BIND TCP/4269" << std::endl;
	}

	serv.run();
}
