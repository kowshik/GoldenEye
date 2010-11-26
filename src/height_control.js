function setHeight(pageName)
{
	

	var minHeights = new Array();
	
	minHeights['home']=1240;
	minHeights['team']=740;
	minHeights['developers']=940;
	
	if(screen.height < minHeights[pageName])
	{
		document.getElementById('content').style.height=''+minHeights[pageName]+'px';
	}
	else
	{
			document.getElementById('content').style.height=screen.height+'px';
	}
	
}
