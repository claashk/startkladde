#include "src/gui/widgets/NotificationsLayout.h"

#include <iostream>
#include <cmath>
#include <iomanip>

#include "src/util/math.h"

NotificationsLayout::NotificationsLayout ()
{
}

NotificationsLayout::~NotificationsLayout ()
{
}

void NotificationsLayout::add (NotificationWidget *widget)
{
	Node node;

	node.widget=widget;
	node.visible=false;

	_nodes.insert (widget, node);
}

void NotificationsLayout::remove (const NotificationWidget *widget)
{
	_nodes.remove (widget);
}

void NotificationsLayout::setWidgetPosition (const NotificationWidget *widget, const QPointF &position)
{
	Node &node=_nodes[widget];

	node.arrowPosition=position;
	node.visible=true;
}

void NotificationsLayout::setWidgetInvisible (const NotificationWidget *widget)
{
	Node &node=_nodes[widget];

	node.visible=false;
}

double NotificationsLayout::overlapValue (QList<LayoutNode> &nodes, int index, int spacing)
{
	// The first item can overlap with the top. All other items can overlap with
	// the previous item (we'll ignore the fact that all items can overlap with
	// the top or with before-previous items).

	double y=nodes[index].y;

	double minY;
	if (index==0)
		minY=0;
	else
		minY=nodes[index-1].y + nodes[index-1].h;

	minY+=spacing;

	if (y<minY)
		return (minY-y);
	else
		return 0;
}

int NotificationsLayout::largestOverlap (QList<LayoutNode> &nodes, int spacing)
{
	// Start with the overlap of the first item.
	int maxIndex=-1;
	double maxOverlap=0;

	for (int i=0, n=nodes.size (); i<n; ++i)
	{
		double overlap=overlapValue (nodes, i, spacing);

		// Weighting
		if (i>0)
			overlap*=(sqrt(2)/2);

		if (overlap>maxOverlap)
		{
			maxIndex=i;
			maxOverlap=overlap;
		}
	}

	return maxIndex;
}

void NotificationsLayout::doLayout(QList<LayoutNode> &nodes)
{
	const int maxIterations=150;
	const int spacing=1;

	if (nodes.isEmpty ())
		return;

	qSort (nodes);

	//std::cout << "============= Do layout for " << nodes.size () << " items =====" << std::endl;

	int iteration;
	for (iteration=0; iteration<maxIterations; ++iteration)
	{
		//std::cout << std::setprecision(3) << "Iteration " << iteration << ", Nodes: ";
		//for (int i=0, n=nodes.size (); i<n; ++i)
		//	std::cout << i << ": " << nodes[i].y << " + " << nodes[i].h << ", ";

		int index=largestOverlap (nodes, spacing);

		if (index<0)
			break;

		double overlap=overlapValue (nodes, index, spacing);

		if (overlap < 1e-3)
			break;

		//std::cout << "largest overlap: " << overlap << " at " << index << std::endl;

		if (index==0)
		{
			nodes[index].y+=overlap;
		}
		else
		{
			nodes[index]  .y+=overlap/2;
			nodes[index-1].y-=overlap/2;
		}
	}

	//std::cout << "Done after " << iteration << " iterations" << std::endl;
}

void NotificationsLayout::layout ()
{
	QList<Node> visibleNodes;

	// Iterate over all nodes. Make a list of visible nodes and hide all
	// invisible nodes.
	QHashIterator<const NotificationWidget *, Node> i (_nodes);
	while (i.hasNext ())
	{
		i.next ();

		const Node &node=i.value ();

		if (node.visible)
			visibleNodes.append (node);
		else
			node.widget->hide ();
	}

	// Create layout nodes for the visible nodes
	QList<LayoutNode> layoutNodes;
	for (int i=0, n=visibleNodes.size (); i<n; ++i)
	{
		LayoutNode layoutNode;
		layoutNode.node=&visibleNodes[i];
		layoutNode.y=visibleNodes[i].widget->defaultBubblePosition (visibleNodes[i].arrowPosition).y ();
		layoutNode.h=visibleNodes[i].widget->bubbleGeometry ().height ();
		layoutNodes.append (layoutNode);
	}

	doLayout (layoutNodes);

	// Apply the layout
	foreach (const LayoutNode &layoutNode, layoutNodes)
	{
		QPointF arrowPosition=layoutNode.node->arrowPosition;
		QPointF bubblePosition=layoutNode.node->widget->defaultBubblePosition (arrowPosition);
//		std::cout << layoutNode.y << std::endl;
		bubblePosition.setY (layoutNode.y);
		layoutNode.node->widget->moveTo (arrowPosition, bubblePosition);
		layoutNode.node->widget->show ();
	}


}
