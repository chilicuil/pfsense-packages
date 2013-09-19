<?php
/* $Id$ */
/* ========================================================================== */
/*
    smokeping-agent.inc
    Copyright (C) 2013 Javier Lopez

    All rights reserved.            
			                                                                  */
/* ========================================================================== */
/*
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
                                                                              */
/* ========================================================================== */
require_once("util.inc");
require_once("functions.inc");
require_once("pkg-utils.inc");
require_once("globals.inc");
require_once("guiconfig.inc");


function install_smokeping_agent(){
	conf_mount_rw();
	exec("ln -s /usr/local/sbin/fping /usr/bin/fping");
	conf_mount_ro();
}

function uninstall_smokeping_agent(){
        conf_mount_rw();
	exec("/usr/sbin/service smokeping onestop");
	unlink_if_exists("/usr/local/etc/rc.d/smokeping");
        conf_mount_ro();
}

function validate_input_smokeping_agent($post,&$input_errors){
        conf_mount_rw();
	if ($post['agentenabled']=="on"){
		if (preg_match("/\w+/", $post['master'])) {
        		exec("/usr/bin/fetch {$post['master']}");

			$fsmokeping = './smokeping.cgi';

			if (file_exists($fsmokeping)) {
				exec("/bin/rm $fsmokeping");
			} else {
				$input_errors[]="Couldn't resolve \"{$post['master']}\", the URL is down or contains an invalid address.";
			}
		} else {
			$input_errors[]="Master field is required.";
		}
		
		if($post['slavesecret'] and $post['slavesecretagain']) {
			if($post['slavesecret'] != $post['slavesecretagain']) {
				$input_errors[] = "The provided passwords did not match.";
			}
		} else {
			$input_errors[] = "You must provide (and confirm) smokeping's password.";
		}
	}	
        conf_mount_ro();
}

function sync_smokeping_agent(){
	global $config, $g;
	conf_mount_rw();

	/* check smokeping agent settings*/
	if (is_array($config['installedpackages']['smokepingagent'])) {
		if ($config['installedpackages']['smokepingagent']['config'][0]['agentenabled']) {
			$smokeping_agent_config = $config['installedpackages']['smokepingagent']['config'][0];
			exec("/bin/echo " . $smokeping_agent_config['slavesecret'] . " > /usr/local/etc/smokeping/slavesecrets.conf");
			exec("/bin/chmod 700 /usr/local/etc/smokeping/slavesecrets.conf");
			exec("/usr/sbin/chown smokeping:smokeping /usr/local/etc/smokeping/slavesecrets.conf");
			exec("sed -i -e \"s:NO:YES:g\" /usr/local/etc/rc.d/smokeping");
			exec("sed -i -e \"/smokeping_master=/ s,=.*,={$smokeping_agent_config['master']},\" /usr/local/etc/rc.d/smokeping");
			if($smokeping_agent_config['slavename']) {
				exec("sed -i -e \"/slavename=/ s:=.*:={$smokeping_agent_config['slavename']}:\" /usr/local/etc/rc.d/smokeping");
			}

			if($smokeping_agent_config['master'] and $smokeping_agent_config['slavesecret']) {
				exec("/usr/sbin/service smokeping start", $output, $status);
				$savemsg = get_std_save_message($status);
				if ($savemsg) {
					print_info_box($savemsg);
				} else {
					$savemsg='Smokeping agent was not able to start, verify this Smokeping Agent has an entry in the <a href="http://oss.oetiker.ch/smokeping/doc/smokeping_master_slave.en.html">Master server';
					print_info_box($savemsg);
				}
			}
		} else {
			exec("/usr/sbin/service smokeping onestop");
		}
	}
	conf_mount_ro();
}

function status_smokeping_agent() {
	$rcsmokeping = '/usr/local/etc/rc.d/smokeping';
	if (file_exists($rcsmokeping)) {
		exec("service smokeping onestatus", $output, $status);
		if ($status != 0) {
			return false;
		}
	} else {
                return false;
	}
        return true;
}
?>
