function httpREQ(_cmd, _url, _data, _callback)
{
	var req = false;

	if (window.XMLHttpRequest) 
	{ // Mozilla, Safari,...
		req = new XMLHttpRequest();
    if (req.overrideMimeType) 
    {
    	req.overrideMimeType('text/xml');
		}
	}
	else if (window.ActiveXObject) 
	{ // IE
		try 
		{
    	req = new ActiveXObject("Msxml2.XMLHTTP");
		}
		catch (e) 
		{
			try 
			{
				req = new ActiveXObject("Microsoft.XMLHTTP");
			}
			catch (e) {}
		}
	}

	if (!req) 
	{
		alert('Giving up :( Cannot create an XMLHTTP instance');
		return false;
	}

	req.onreadystatechange = function(){ 
			if (req.readyState == 4)
			{
				if (req.status == 200) 
				{
					_callback(req.responseText);
		}
	}
	};
	req.open(_cmd, _url, true);
	req.send(_data);
}

function httpGET(url, callback)
{
	httpREQ('GET', url, null, callback);
}

function httpPOST(url, params, callback) 
{
	httpREQ('POST', url, params, callback);
}
