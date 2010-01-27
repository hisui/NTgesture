


function trace(msg)
{
	chrome.extension.sendRequest({$:"trace", message:msg});
}

function preventContextMenu()
{
	window.addEventListener("contextmenu",
	function (e)
	{
		e.preventDefault();
		window.removeEventListener("contextmenu", arguments.callee, false);
	}, false);
}



chrome.extension.onRequest.addListener(function (request, sender, sendResponse)
{

	switch(request.$) {
	case "preventContextMenu":
		preventContextMenu();
		break;
	}
});


