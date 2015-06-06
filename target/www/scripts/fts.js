var nReflashTimerID = 0;
var nReflashInterval= 5;
var _nLastUpdateTime = 0;
var page;

function removeFrame()
{
    return  (elem=document.getElementById('content-main')).parentNode.removeChild(elem);
}

function createFrame(title)
{
    titleElem = document.createElement('h2');
    titleElem.class='section_title';
    titleElem.innerHTML=title;
    
    frame = document.createElement('div');
    frame.setAttribute('id','content-main');
    frame.setAttribute('class', 'columns');
    
    frame.appendChild(titleElem);

    return  frame;
}

function createDevInfo(data)
{
	var section = document.createElement('div');
	section.setAttribute('class', 'content_section');
	
	var title = document.createElement('h3');
	title.innerHTML='Product Informations';
	section.appendChild(title);

	table = document.createElement('table');
    for(i = 0 ; i < data.descs.length ; i++)
    {
        row = table.insertRow(i);
        cell = row.insertCell(0);
        cell.setAttribute('class','devinfo_first');
        cell.innerHTML = data.descs[i].title + ' :';
        cell = row.insertCell(1);
        cell.innerHTML = data.descs[i].value;
    }

    section.appendChild(table);
    
    return  section;
}

function    createStatusGroup(group)
{
	section = document.createElement('div');
	section.setAttribute('class', 'content_section');
	
	title = document.createElement('h3');
	title.innerHTML=group.name;
	section.appendChild(title);

	table = document.createElement('table');
    row = table.insertRow(0);
    for(i = 0 ; i < group.fields.length ; i++)
    {
        cell = row.insertCell(i);
        cell.setAttribute('class','title left');
        cell.innerHTML = group.fields[i];
    }

    for(i = 0 ; i < group.objects.length ; i++)
    {
        row = table.insertRow(i+1);
        switch(group.name)
        {
        case    'DO':   
        case    'RL':   
            createStatusRowDO(row, group.objects[i], onApplyCtrl); 
            break;
            
        default:        
            createStatusRowDefault(row, group.objects[i]); 
            break;
        }
        onUpdateObject(group.objects[i]);
    }    
    
    section.appendChild(table);
    
	return	section;
}

function    createConfigGroup(group)
{
	section = document.createElement('div');
	section.setAttribute('class', 'content_section');
	
	title = document.createElement('h3');
	title.innerHTML=group.name;
	section.appendChild(title);

	table = document.createElement('table');
    row = table.insertRow(0);
    for(i = 0 ; i < group.fields.length ; i++)
    {
        cell = row.insertCell(i);
        cell.setAttribute('class','title left');
        cell.innerHTML = group.fields[i];
    }

    for(i = 0 ; i < group.objects.length ; i++)
    {
        row = table.insertRow(i+1);
        switch(group.name)
        {
        case    'DS18B20':   
            createConfigRowDS18B20(row, group.objects[i]); 
            break;
            
        default:        
            createConfigRowDefault(row, group.objects[i]); 
            break;
        }
    }    
    section.appendChild(table);
    
	return	section;
}


function    createSystemGroup(name, group)
{
	section = document.createElement('div');
	section.setAttribute('class', 'content_section');
	
	title = document.createElement('h3');
	title.innerHTML=name;
	section.appendChild(title);

	table = document.createElement('table');
    switch(name)
    {
    case    'Network':
        {
            type_opts   = ['static', 'dhcp'];
        
            row = table.insertRow(0);
            cell = row.insertCell(0);
            cell.setAttribute('class','system_first');
            cell.innerHTML = 'IP Address :';            
            cell = row.insertCell(1);
            cell.innerHTML = group.mac;
            
            row = table.insertRow(1);            
            cell = row.insertCell(0);
            cell.setAttribute('class','system_first');
            cell.innerHTML = 'Type :';            
            createSelectCell(row, 1, 'net_type', type_opts, group.type);
            
            row = table.insertRow(2);            
            cell = row.insertCell(0);
            cell.setAttribute('class','system_first');
            cell.innerHTML = 'MAC Address :';            
            createEditCell(row, 1, 'ip', group.ip);
            
            row = table.insertRow(3);            
            cell = row.insertCell(0);
            cell.setAttribute('class','system_first');
            cell.innerHTML = 'Subnet Mask :';            
            createEditCell(row, 1, 'mask', group.mask);
            
            row = table.insertRow(4);            
            cell = row.insertCell(0);
            cell.setAttribute('class','system_first');
            cell.innerHTML = 'Default Gateway :';            
            createEditCell(row, 1, 'gateway', group.gateway);
        }            
        break;
        
    case    'Servers':
        {
            for(i = 0 ; i < group.length ; i++)
            {
                row = table.insertRow(i);
                cell = row.insertCell(0);
                cell.setAttribute('class','system_first');
                cell.innerHTML = (i+1) + ' :';            
                cell = row.insertCell(1);
                createEditCell(row, 1, 'server' + i, group[i]);
            }
        }            
        break;
        
    case    'Console':
        {
            baudrate_opts   = ['1200','2400','4800','9600','14400','38400','57600','115200'];
            datebits_opts   = ['5','6','7','8'];
            stopbits_opts   = ['1','1.5','2'];
            parity_opts     = ['none','old','even'];
            flowctrl_opts   = ['none','xon/xoff','RTS/CTS','DSR/DTR'];

            row = table.insertRow(0);
            cell = row.insertCell(0);
            cell.setAttribute('class','system_first');
            cell.innerHTML = 'Baudrate :';            
            cell = row.insertCell(1);
            createSelectCell(row, 1, 'baudrate', baudrate_opts, group.baudrate)

            row = table.insertRow(1);            
            cell = row.insertCell(0);
            cell.setAttribute('class','system_first');
            cell.innerHTML = 'Data Bits :';            
            createSelectCell(row, 1, 'databits', datebits_opts, group.databits)
            
            row = table.insertRow(2);            
            cell = row.insertCell(0);
            cell.setAttribute('class','system_first');
            cell.innerHTML = 'Stop Bits :';            
            createSelectCell(row, 1, 'stopbits', stopbits_opts, group.stopbits)
            
            row = table.insertRow(3);            
            cell = row.insertCell(0);
            cell.setAttribute('class','system_first');
            cell.innerHTML = 'Parity Bits :';            
            createSelectCell(row, 1, 'paritybits', parity_opts, group.paritybits)

            row = table.insertRow(4);            
            cell = row.insertCell(0);
            cell.setAttribute('class','system_first');
            cell.innerHTML = 'Flow Control :';            
            createSelectCell(row, 1, 'flowcontrol', flowctrl_opts, group.flowcontrol)
        }            
        break;
        
    }
        
    section.appendChild(table);
    
	return	section;
}

function createSystemCtrls()
{
	table = document.createElement('table');
    row = table.insertRow(-1);
    
    createButtonCell(row, 0, 'factory_reset', null, 'Factory Reset', false, onFactoryReset);
    createButtonCell(row, 1, 'reboot', null, 'Restart', false, onReboot);
    createButtonCell(row, 2, 'apply', null, 'Apply', false, onApplySystem);
    
	section = document.createElement('div');
    section.appendChild(table);
    
    return  section;
}
function createStatusRowDefault(row, obj)
{

    createTextCell(row, 0, obj.id + '_' + 'oid',    obj.id);
    createTextCell(row, 1, obj.id + '_' + 'name',   obj.name);
    createTextCell(row, 2, obj.id + '_' + 'value',  obj.value);
    createTextCell(row, 3, obj.id + '_' + 'status', getStatusString(obj.status));
}

function createStatusRowDO(row, obj, onClick)
{
    createTextCell(row, 0, obj.id + '_' + 'oid',    obj.id);
    createTextCell(row, 1, obj.id + '_' + 'name',   obj.name);
    createTextCell(row, 2, obj.id + '_' + 'value',  obj.value);
    createTextCell(row, 3, obj.id + '_' + 'status', getStatusString(obj.status));
    createButtonCell(row, 4, obj.id + '_' + 'ctrl', obj.id, obj.cvalue, (obj.status.search('E') < 0), onClick);
}


function createConfigRowDS18B20(row, obj)
{
    createCell(row, 0, obj.id + '_' + 'oid',    obj.id);
    createEditCell(row, 1, obj.id + '_' + 'name',   obj.name);
    createCheckboxCell(row, 2, obj.id + '_' + 'value',  (obj.status.search('E') < 0));
}

function createConfigRowDefault(row, obj, onClick)
{
    createTextCell(row, 0, obj.id + '_' + 'oid',    obj.id);
    createEditCell(row, 1, obj.id + '_' + 'name',   obj.name);
    createCheckboxCell(row, 2, obj.id + '_' + 'enable',  (obj.enable == 'true'));
}


/*******************************************************************************
 * Page Control Events
 ******************************************************************************/

function onLoadPage(page)
{
	httpGET('/request.cgi?cmd=view&page=' + page, onRespLoadPage);
}

function onRespLoadPage(resp)
{
    page = JSON.parse(resp, replacer);   

    product.innerHTML = page.model;

    frame = createFrame(page.title);
        
    if (typeof(page.product_info) != 'undefined')
    {
        frame.appendChild(document.createElement('hr'));
        frame.appendChild(createDevInfo(page.product_info));
    }
    
    if (typeof(page.groups) != 'undefined')
    {
        for(objIdx = 0 ; objIdx < page.groups.length ; objIdx++)
        {            
            frame.appendChild(document.createElement('hr'));
            switch(page.type)
            {
            case 'status':
                frame.appendChild(createStatusGroup(page.groups[objIdx]));
                break;
            
            case 'config':
                frame.appendChild(createConfigGroup(page.groups[objIdx]));
                break;
            }
        }

        if (page.type == 'config')
        {
            frame.appendChild(document.createElement('hr'));
            frame.appendChild(document.createElement('br'));
            
            btnApply = document.createElement('input');
            btnApply.type = 'button';
            btnApply.value = 'Apply';
            btnApply.onclick= onApplyConfig;
            frame.appendChild(btnApply);
        }
    }
    
    if (typeof(page.system) != 'undefined')
    {
        if (typeof(page.system.network) != 'undefined')
        {
            frame.appendChild(document.createElement('hr'));
            frame.appendChild(createSystemGroup('Network', page.system.network))
        }
        
        if (typeof(page.system.servers) != 'undefined')
        {
            frame.appendChild(document.createElement('hr'));
            frame.appendChild(createSystemGroup('Servers', page.system.servers))
        }
        
        if (typeof(page.system.console) != 'undefined')
        {
            frame.appendChild(document.createElement('hr'));
            frame.appendChild(createSystemGroup('Console', page.system.console))
        }
    
        frame.appendChild(document.createElement('hr'));
        frame.appendChild(createSystemCtrls());
    }
    
    if (typeof(page.LUT) != 'undefined')
    {
        _nLastUpdateTime = page.LUT;
    }
    if (typeof(page.RI) != 'undefined')
    {
        nReflashInterval = page.RI;
        if (nReflashTimerID != 0)
        {
            clearInterval(nReflashTimerID);
            nReflashTimerID = 0;
        }
        nReflashTimerID = self.setInterval(function(){onReflashRequest(_nLastUpdateTime)}, nReflashInterval*1000);
    }
    else
    {
        clearInterval(nReflashTimerID);
        nReflashInterval = 0;
    }
    removeFrame();
    document.getElementById('content-wrapper').appendChild(frame);        
}

function onReflashRequest(nLastUpdateTime)
{
    if (nLastUpdateTime == _nLastUpdateTime)
    {
        var msg = 'cmd=update';
        msg += '&lut=' + _nLastUpdateTime;
        httpGET('/request.cgi?' + msg, onReflashResponse);
    }
}

function onReflashResponse(data)
{
    try
    {
        resp = JSON.parse(data, replacer);   
   
        for(i = 0 ; i < resp.objects.length; i++)
        {
            onUpdateObject(resp.objects[i]);
        }
        
        if(resp.LUT != 'undefined')
        {
            _nLastUpdateTime = resp.LUT;
        }
    }
    catch(e)
    {
    }
}

function onUpdateObject(object)
{
    obj = document.getElementById(object.id + '_name');
    if (obj != null)
    {
        obj.innerHTML = object.name;
    }

    obj = document.getElementById(object.id + '_value');
    if (obj != null)
    {
        obj.innerHTML = object.value;
        
        if(object.status.search('V') >= 0)
        {
            obj.setAttribute('class', 'default');
        }
        else
        {
            obj.setAttribute('class', 'value_na');
        }
    }
    
    obj = document.getElementById(object.id + '_status');
    if (obj != null)
    {
        obj.innerHTML = getStatusString(object.status);
    }
    
    obj = document.getElementById(object.id + '_ctrl');
    if (obj != null)
    {
        if (object.value.toUpperCase() == 'ON')
        {
            obj.value = 'OFF';
        }
        else if (object.value.toUpperCase() == 'OFF')
        {
            obj.value = 'ON';
        }
        else if (object.value.toUpperCase() == 'CLOSE')
        {
            obj.value = 'OPEN';
        }
        else if (object.value.toUpperCase() == 'OPEN')
        {
            obj.value = 'CLOSE';
        }
    }
}


/*******************************************************************************
 * Status Control Events
 ******************************************************************************/

function onApplyCtrl(oid, value)
{
    var msg = 'cmd=ctrl';
    var params = oid.split('_');
    
    obj = document.getElementById(oid);
    msg += '&oid='+ params[0];
    msg += '&value=' + obj.value.toLowerCase();
        
    httpGET('/request.cgi?' + msg, onRespCtrl);

}

function onRespCtrl(data)
{
	try
	{
        ret  = JSON.parse(data, replacer);   
   
        if (ret.ret == 'success')
        {
            obj = document.getElementById(ret.oid+'_ctrl');
            if (obj != 'undefined')
            {
                obj.value = ret.ctrl.toUpperCase();
            }
        
            obj = document.getElementById(ret.oid+'_value');
            if (obj != 'undefined')
            {
                obj.innerHTML = ret.value.toUpperCase();
            }
        
            alert('Success');
        }
        else
        {
            alert('Failed');
        }
	}
	catch(e)
	{
		
	}
}

/*******************************************************************************
 * Point Configuration Events
 ******************************************************************************/
function onApplyConfig()
{
	var data = 'cmd=config';

    for(i = 0 ; i < page.groups.length ; i++)
	{   
        for (j = 0 ; j < page.groups[i].objects.length ; j++)
        {
			id_name = page.groups[i].objects[j].id +'_' + page.groups[i].fields[1].toLowerCase();
			id_used = page.groups[i].objects[j].id +'_' + page.groups[i].fields[2].toLowerCase();
			obj_name = document.getElementById(id_name);
			obj_used = document.getElementById(id_used);
		
			data += '&' + id_name +'=' + obj_name.value;
			data += '&' + id_used +'=' + obj_used.checked;
		}
	}

	httpPOST('/request.cgi?', data, onRespConfig);	
}

function onRespConfig(data)
{
	if (data == 'SUCCESS')
	{
		alert(data);
		
	}
}


/*******************************************************************************
 * System Control Events
 ******************************************************************************/
 function onApplySystem()
{
	var	msg = 'cmd=system';

    try
    {

        if (!IsValidIP(ip, false))
        {
                alert('Missing or invalid IP address.', ipaddr);
                return;
        }
        
        if (!IsValidIP(mask, false))
        {
                alert('Missing or invalid Subnet Mask.', netmask);
                return;
        }
        
        if (!IsValidIP(gateway, false))
        {
                alert('Missing or invalid Gateway IP address.', gateway);
                return;
        }
        
        if (!IsValidIP(server0, true))
        {
                alert('Missing or invalid Server1 IP address.', server_ipaddr1);
                return;
        }
        
        if (!IsValidIP(server1, true))
        {
                alert('Missing or invalid Server2 IP address.', server_ipaddr2);
                return;
        }
        
        msg += '&type='         + net_type.value;
        msg += '&ip='           + ip.value;
        msg += '&netmask='      + mask.value;
        msg += '&gateway='      + gateway.value;
        msg += '&server1='      + server0.value;
        msg += '&server2='      + server1.value;
/*        
        msg += '&baudrate='     + baudrate.value;
        msg += '&databits='     + databits.value;
        msg += '&stopbits='     + stopbits.value;
        msg += '&paritybits='   + paritybits.value;
        msg += '&flowcontrol='  + flowcontrol.value;
*/      
        
        httpPOST('/request.cgi?', msg, onRespSystem);
    }
    catch(e)
    {}    
}

function onRespSystem(data)
{
		alert(data);
}

function onChangeNetType()
{
	if (net_type.value == 'static')
	{
		ip.readOnly	= false;
		ip.style.color='#333';
		mask.readOnly= false; 
		mask.style.color='#333';
		gateway.readOnly= false;
		gateway.style.color='#333';
	}
	else
	{
		ip.readOnly	= true;
		ip.style.color='#D0D0D0';
		mask.readOnly= true; 
		mask.style.color='#D0D0D0';
		gateway.readOnly= true;
		gateway.style.color='#D0D0D0';
	}
}
function	onFactoryReset()
{
	var ret = confirm('All data will be deleted. Do you want to continue?');
	if (ret == true)
	{
		httpGET('/request.cgi?cmd=factory_reset', onFactoryResetResponse);	
		setInterval(function() { window.location.href = '/';}, 2000);
	}
}

function	onFactoryResetResponse(data)
{
}

function onReboot()
{
	var ret = confirm('Reboot now?');
	if (ret == true)
	{
		httpGET('/request.cgi?cmd=reboot', onRebootResponse);	
		setInterval(function() { window.location.href = '/';}, 2000);
  }
}

function	onRebootResponse(data)
{
}



/*******************************************************************************
 * Common Utilities 
 ******************************************************************************/
function GetValue(obj) 
{
  return obj.value.replace(/\s/g, '');
}

function IsValidIP(obj, empty) 
{
	var str = GetValue(obj);
	if (empty && str.length == 0)
	{
		return true;
	}

	var fields = str.match(/^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/);
	if (fields != null) 
	{
		var tmp = fields[1] | fields[2] | fields[3] | fields[4];
		return (tmp < 256) && (empty || tmp > 0);
	} 
	else 
	{
		return false;
	}
}

function getStatusString(status)
{
    if (status.search('E')>= 0)
    {
        if (status.search('V')>= 0)
        {
            return  'RUNNING';
        }
        else
        {
            return  'WARNING';
        }
        
    }
    else
    {
        return  'DISABLED';
    }
}

function createTextCell(row, col, did, data)
{
    cell = row.insertCell(col);        
    cell.setAttribute('class','default');
    cell.innerHTML = data;
    cell.setAttribute('id', did);
    
    return  cell;
}

function createButtonCell(row, col, id, did, value, disabled, onClick)
{
    input = document.createElement('input');
	input.type = 'button';
    input.value = value; 
    input.onclick = function(){onClick(id, value);};
    input.disabled = disabled;
    input.setAttribute('class', 'ctrl_btn');
	input.setAttribute('id', id);
    
    cell = row.insertCell(col);        
    cell.setAttribute('class','default');
    cell.appendChild(input);
}


function createEditCell(row, col, id, value)
{
    input = document.createElement('input');
	input.type = 'text';
    input.value = value; 
	input.setAttribute('id', id);
    
    cell = row.insertCell(col);        
    cell.setAttribute('class','default');
    cell.appendChild(input);
}

function createCheckboxCell(row, col, id, value)
{
    input = document.createElement('input');
	input.type = 'checkbox';
    input.checked = (value == true);
	input.setAttribute('id', id);
    
    cell = row.insertCell(col);        
    cell.setAttribute('class','default');
    cell.appendChild(input);
}

function createSelectCell(row, col, id, options, value)
{
    select = document.createElement('select');
    for(i = 0 ; i < options.length ; i++)
    {
        option = document.createElement('option');
        option.text = options[i];
        select.add(option);
        
        if (options[i] == value)
        select.selectedIndex = i;
    }
    select.setAttribute('id', id);
    select.setAttribute('class','config_second');
    
    cell = row.insertCell(col);        
    cell.appendChild(select);
}

function replacer(key, value) 
{
    if (typeof value === 'number' && !isFinite(value)) 
    {
        return String(value);
    }
    return value;
}

