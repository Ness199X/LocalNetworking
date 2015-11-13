Ayrias Local Networking Plugin
=========================

This plugin is based on code by [@convery](https://github.com/convery) forked from their [opennetplugin](https://github.com/convery/opennetplugin) and contains a full rewrite and general interface changes. It does however retain the same license (LGPL 3.0) as the original repository. Please respect that when iterating on this work. Please check the issues tab for suggested improvements.

Module system
------------------

To extend the abilities of the plugin it can load other DLLs implementing the IServer interface. These modules may be LZ4 compressed and/or AES256 encrypted with a license key. All of this information is entered into a CSV file in the format of `Hostname, Modulename, Licensekey, ` where the hostname may be an IP, modulename may contain ".LZ4" and/or ".AES" to enable deflation and decryption and license can be any string.

Purpose
---------
This plugin is intended to offer developers a way to work off-site where they don't have an internet connection. It may also be used by developers to wrap their existing server solution and protocol to use Ayrias backend for performance testing. It has proved itself useful in anti-malware research and academic research.

Legality
---------

Please be aware that this, like all tools, can be used for both good and evil. Thus we ask nicely that it'll not be used on any system without the users acknowledgement (e.g. via a EULA/TOS) nor outside of academia. Ayria does not accept any responsibility for what third parties do with the code or information contained within this repository nor do we encourage its use outside of developers testing environments.
